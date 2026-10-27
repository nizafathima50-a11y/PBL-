#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lru.h"

#define MAX_REF 200
#define MAX_FRAMES 20
#define OUT_SIZE 50000

typedef enum { FIFO = 0, LRU = 1, OPTIMAL = 2 } Algorithm;

typedef struct {
    int n, f, refs[MAX_REF], frames[MAX_REF][MAX_FRAMES], hit[MAX_REF];
    int hits, faults;
} Result;

typedef struct {
    GtkWidget *pages, *frame_count, *algorithm, *output;
} Widgets;

static const char *algorithm_name(int a)
{
    if (a == FIFO) return "FIFO";
    if (a == LRU) return "LRU";
    return "Optimal";
}

static int parse_refs(const char *s, int out[], int *n)
{
    int count = 0;
    const char *p = s;

    while (*p) {
        char *end;
        long v;

        while (isspace((unsigned char)*p) || *p == ',') p++;
        if (!*p) break;

        v = strtol(p, &end, 10);
        if (end == p || v < 0 || v > 2147483647L ||
            count >= MAX_REF) return 0;

        out[count++] = (int)v;
        p = end;

        if (*p && !isspace((unsigned char)*p) && *p != ',') return 0;
    }

    *n = count;
    return count > 0;
}

static int simulate(const int refs[], int n, int f, int algorithm, Result *r)
{
    int frame[MAX_FRAMES], inserted[MAX_FRAMES];
    int i, j;

    if (!refs || !r || n < 1 || n > MAX_REF ||
        f < 1 || f > MAX_FRAMES) return 0;

    /*
     * Use Wilona's separate LRU module for LRU requests.
     */
    if (algorithm == LRU) {
        LRUResult lr;

        if (!simulate_lru(refs, n, f, &lr)) return 0;

        r->n = lr.n;
        r->f = lr.f;
        r->hits = lr.hits;
        r->faults = lr.faults;

        for (i = 0; i < n; i++) {
            r->refs[i] = lr.refs[i];
            r->hit[i] = lr.hit[i];

            for (j = 0; j < f; j++)
                r->frames[i][j] = lr.frames[i][j];
        }

        return 1;
    }

    r->n = n;
    r->f = f;
    r->hits = 0;
    r->faults = 0;

    for (j = 0; j < f; j++) {
        frame[j] = -1;
        inserted[j] = -1;
    }

    for (i = 0; i < n; i++) {
        int found = 0, slot = -1;

        for (j = 0; j < f; j++) {
            if (frame[j] == refs[i]) {
                found = 1;
                slot = j;
                break;
            }
        }

        r->refs[i] = refs[i];
        r->hit[i] = found;

        if (found) {
            r->hits++;
        } else {
            int empty = -1;
            r->faults++;

            for (j = 0; j < f; j++) {
                if (frame[j] == -1) {
                    empty = j;
                    break;
                }
            }

            if (empty >= 0) {
                slot = empty;
            } else if (algorithm == FIFO) {
                slot = 0;

                for (j = 1; j < f; j++) {
                    if (inserted[j] < inserted[slot])
                        slot = j;
                }
            } else {
                int farthest = -1;
                slot = 0;

                for (j = 0; j < f; j++) {
                    int k, next = -1;

                    for (k = i + 1; k < n; k++) {
                        if (refs[k] == frame[j]) {
                            next = k;
                            break;
                        }
                    }

                    if (next == -1) {
                        slot = j;
                        break;
                    }

                    if (next > farthest) {
                        farthest = next;
                        slot = j;
                    }
                }
            }

            frame[slot] = refs[i];
            inserted[slot] = i;
        }

        for (j = 0; j < f; j++)
            r->frames[i][j] = frame[j];
    }

    return 1;
}

static void append(char *out, size_t cap, const char *s)
{
    size_t used = strlen(out);

    if (used < cap - 1)
        strncat(out, s, cap - used - 1);
}

static void format_result(const char *name, const Result *r,
                          char *out, size_t cap)
{
    int i, j;
    char line[512];

    snprintf(line, sizeof(line),
             "%s\nReference | Frames after request | Result\n", name);
    append(out, cap, line);
    append(out, cap, "-----------------------------------------\n");

    for (i = 0; i < r->n; i++) {
        snprintf(line, sizeof(line), "%9d | ", r->refs[i]);
        append(out, cap, line);

        for (j = 0; j < r->f; j++) {
            if (r->frames[i][j] == -1)
                snprintf(line, sizeof(line), "- ");
            else
                snprintf(line, sizeof(line), "%d ", r->frames[i][j]);

            append(out, cap, line);
        }

        append(out, cap, r->hit[i] ? "| HIT\n" : "| PAGE FAULT\n");
    }

    snprintf(line, sizeof(line),
        "\nTotal hits: %d\nTotal page faults: %d\n"
        "Hit ratio: %.2f%%\nFault ratio: %.2f%%\n\n",
        r->hits, r->faults,
        100.0 * r->hits / r->n,
        100.0 * r->faults / r->n);

    append(out, cap, line);
}

static void show_output(Widgets *w, const char *s)
{
    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(w->output)), s, -1);
}

static void run_clicked(GtkButton *button, gpointer data)
{
    Widgets *w = data;
    int refs[MAX_REF], n = 0, selected;
    const char *frame_text;
    char *end = NULL;
    long f;
    char *out = calloc(OUT_SIZE, 1);

    (void)button;

    if (!out) {
        show_output(w, "Could not allocate memory for results.");
        return;
    }

    if (!parse_refs(gtk_editable_get_text(GTK_EDITABLE(w->pages)),
                    refs, &n)) {
        show_output(w,
            "Input error: enter 1-200 non-negative page numbers "
            "separated by spaces or commas.");
        free(out);
        return;
    }

    frame_text = gtk_editable_get_text(GTK_EDITABLE(w->frame_count));
    f = strtol(frame_text, &end, 10);

    if (end == frame_text || *end || f < 1 || f > MAX_FRAMES) {
        show_output(w,
            "Input error: frame count must be a whole number from 1 to 20.");
        free(out);
        return;
    }

    selected = (int)gtk_drop_down_get_selected(GTK_DROP_DOWN(w->algorithm));

    if (selected == 3) {
        Result results[3];
        int a, best = 0;

        for (a = 0; a < 3; a++) {
            if (!simulate(refs, n, (int)f, a, &results[a])) {
                show_output(w, "Simulation failed.");
                free(out);
                return;
            }

            if (results[a].faults < results[best].faults)
                best = a;
        }

        snprintf(out, OUT_SIZE,
            "COMPARISON SUMMARY\n"
            "FIFO: %d faults, %d hits\n"
            "LRU: %d faults, %d hits\n"
            "Optimal: %d faults, %d hits\n"
            "Lowest fault count in this run: %s\n\n",
            results[0].faults, results[0].hits,
            results[1].faults, results[1].hits,
            results[2].faults, results[2].hits,
            algorithm_name(best));

        for (a = 0; a < 3; a++) {
            char *section = calloc(12000, 1);

            if (section) {
                format_result(algorithm_name(a), &results[a],
                              section, 12000);
                append(out, OUT_SIZE, section);
                append(out, OUT_SIZE,
                       "========================================\n\n");
                free(section);
            }
        }
    } else {
        Result r;

        if (!simulate(refs, n, (int)f, selected, &r)) {
            show_output(w, "Simulation failed.");
            free(out);
            return;
        }

        format_result(algorithm_name(selected), &r, out, OUT_SIZE);
    }

    show_output(w, out);
    free(out);
}

static void activate(GtkApplication *app, gpointer unused)
{
    GtkWidget *window, *box, *label, *row, *button, *scroll;
    Widgets *w = g_new0(Widgets, 1);
    const char *choices[] = {"FIFO", "LRU", "Optimal", "Compare all", NULL};

    (void)unused;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Page Replacement Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 680);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 9);
    gtk_widget_set_margin_top(box, 16);
    gtk_widget_set_margin_bottom(box, 16);
    gtk_widget_set_margin_start(box, 16);
    gtk_widget_set_margin_end(box, 16);
    gtk_window_set_child(GTK_WINDOW(window), box);

    label = gtk_label_new("Operating Systems - Page Replacement Simulator");
    gtk_widget_add_css_class(label, "title-1");
    gtk_box_append(GTK_BOX(box), label);

    label = gtk_label_new(
        "Page-reference string (spaces or commas between numbers)");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), label);

    w->pages = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(w->pages), "7 0 1 2 0 3 0 4 2 3");
    gtk_box_append(GTK_BOX(box), w->pages);

    label = gtk_label_new("Number of frames (1-20)");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), label);

    w->frame_count = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(w->frame_count), "3");
    gtk_box_append(GTK_BOX(box), w->frame_count);

    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(box), row);

    w->algorithm = gtk_drop_down_new_from_strings(choices);
    gtk_widget_set_hexpand(w->algorithm, TRUE);
    gtk_box_append(GTK_BOX(row), w->algorithm);

    button