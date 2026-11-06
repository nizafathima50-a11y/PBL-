#include "gui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SystemState state;

static GtkWidget *process_entry;
static GtkWidget *resource_entry;
static GtkWidget *recovery_entry;
static GtkWidget *total_entry[MAX_RESOURCES];
static GtkWidget *allocation_entry[MAX_PROCESSES][MAX_RESOURCES];
static GtkWidget *maximum_entry[MAX_PROCESSES][MAX_RESOURCES];
static GtkWidget *request_process_entry;
static GtkWidget *request_entry[MAX_RESOURCES];
static GtkWidget *matrix_box;
static GtkWidget *output_view;
static GtkWidget *main_window;

static int configured = 0;
static int data_applied = 0;

static void request_resources_clicked(
    GtkButton *button,
    gpointer user_data
);

static void draw_arrow(
    cairo_t *cr,
    double x1,
    double y1,
    double x2,
    double y2,
    double red,
    double green,
    double blue,
    const char *label
)
{
    double angle = atan2(y2 - y1, x2 - x1);
    const double arrow_length = 9.0;
    const double arrow_angle = 0.45;

    cairo_set_source_rgb(cr, red, green, blue);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);

    cairo_move_to(cr, x2, y2);
    cairo_line_to(
        cr,
        x2 - arrow_length * cos(angle - arrow_angle),
        y2 - arrow_length * sin(angle - arrow_angle)
    );
    cairo_line_to(
        cr,
        x2 - arrow_length * cos(angle + arrow_angle),
        y2 - arrow_length * sin(angle + arrow_angle)
    );
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_set_font_size(cr, 12.0);
    cairo_move_to(cr, (x1 + x2) / 2.0, (y1 + y2) / 2.0 - 4.0);
    cairo_show_text(cr, label);
}

static void draw_resource_graph(
    GtkDrawingArea *area,
    cairo_t *cr,
    int width,
    int height,
    gpointer user_data
)
{
    const double process_x = 110.0;
    const double resource_x = width - 125.0;
    const double process_radius = 23.0;
    const double resource_half_width = 58.0;
    const double top = 72.0;
    const double bottom = 48.0;
    char label[80];

    (void)area;
    (void)user_data;

    cairo_set_source_rgb(cr, 0.15, 0.15, 0.15);
    cairo_set_font_size(cr, 14.0);
    cairo_move_to(cr, process_x - 35.0, 30.0);
    cairo_show_text(cr, "Processes");
    cairo_move_to(cr, resource_x - 85.0, 30.0);
    cairo_show_text(cr, "Resources (available / total)");

    for (int i = 0; i < state.process_count; i++) {
        double process_y = top +
            (i + 0.5) * (height - top - bottom) / state.process_count;

        for (int j = 0; j < state.resource_count; j++) {
            double resource_y = top +
                (j + 0.5) * (height - top - bottom) /
                state.resource_count;

            if (state.allocation[i][j] > 0) {
                snprintf(
                    label,
                    sizeof(label),
                    "%d",
                    state.allocation[i][j]
                );
                draw_arrow(
                    cr,
                    resource_x - resource_half_width,
                    resource_y - 5.0,
                    process_x + process_radius,
                    process_y - 5.0,
                    0.12, 0.38, 0.72,
                    label
                );
            }

            if (state.request[i][j] > 0) {
                snprintf(label, sizeof(label), "%d", state.request[i][j]);
                draw_arrow(
                    cr,
                    process_x + process_radius,
                    process_y + 5.0,
                    resource_x - resource_half_width,
                    resource_y + 5.0,
                    0.82, 0.35, 0.12,
                    label
                );
            }
        }
    }

    for (int i = 0; i < state.process_count; i++) {
        double process_y = top +
            (i + 0.5) * (height - top - bottom) / state.process_count;

        cairo_set_source_rgb(cr, 0.88, 0.93, 0.99);
        cairo_arc(cr, process_x, process_y, process_radius, 0.0, 2.0 * G_PI);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.12, 0.30, 0.52);
        cairo_set_line_width(cr, 2.0);
        cairo_stroke(cr);

        snprintf(label, sizeof(label), "P%d", i);
        cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
        cairo_set_font_size(cr, 13.0);
        cairo_move_to(cr, process_x - 11.0, process_y + 5.0);
        cairo_show_text(cr, label);
    }

    for (int j = 0; j < state.resource_count; j++) {
        double resource_y = top +
            (j + 0.5) * (height - top - bottom) / state.resource_count;

        cairo_set_source_rgb(cr, 0.91, 0.96, 0.88);
        cairo_rectangle(
            cr,
            resource_x - resource_half_width,
            resource_y - 22.0,
            2.0 * resource_half_width,
            44.0
        );
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.25, 0.48, 0.18);
        cairo_set_line_width(cr, 2.0);
        cairo_stroke(cr);

        snprintf(
            label,
            sizeof(label),
            "R%d (%d/%d)",
            j,
            state.available[j],
            state.total[j]
        );
        cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
        cairo_set_font_size(cr, 13.0);
        cairo_move_to(cr, resource_x - 38.0, resource_y + 5.0);
        cairo_show_text(cr, label);
    }

    cairo_set_source_rgb(cr, 0.12, 0.38, 0.72);
    cairo_set_font_size(cr, 12.0);
    cairo_move_to(cr, 20.0, height - 14.0);
    cairo_show_text(cr, "Blue: allocated resource -> process");
    cairo_set_source_rgb(cr, 0.82, 0.35, 0.12);
    cairo_move_to(cr, width / 2.0, height - 14.0);
    cairo_show_text(cr, "Orange: outstanding process -> resource request");
}

static void show_message(const char *message)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));

    gtk_text_buffer_set_text(buffer, message, -1);
}

static void append_message(const char *message)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    GtkTextIter end;

    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, message, -1);
}

static GtkWidget *make_entry(const char *text)
{
    GtkWidget *entry = gtk_entry_new();

    gtk_editable_set_text(GTK_EDITABLE(entry), text);
    gtk_widget_set_size_request(entry, 65, -1);

    return entry;
}

static GtkWidget *make_section_label(const char *text)
{
    GtkWidget *label = gtk_label_new(text);

    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_add_css_class(label, "heading");

    return label;
}

static bool read_integer(GtkWidget *entry, int *value)
{
    const char *text;
    char *end = NULL;
    long number;

    if (entry == NULL || value == NULL) {
        return false;
    }

    text = gtk_editable_get_text(GTK_EDITABLE(entry));

    if (text == NULL || *text == '\0') {
        return false;
    }

    number = strtol(text, &end, 10);

    if (end == text || *end != '\0' ||
        number < 0 || number > 1000000) {
        return false;
    }

    *value = (int)number;
    return true;
}

static void clear_matrix_widgets(void)
{
    GtkWidget *child;

    while ((child = gtk_widget_get_first_child(matrix_box)) != NULL) {
        gtk_box_remove(GTK_BOX(matrix_box), child);
    }

    memset(total_entry, 0, sizeof(total_entry));
    memset(allocation_entry, 0, sizeof(allocation_entry));
    memset(maximum_entry, 0, sizeof(maximum_entry));

    configured = 0;
    data_applied = 0;
}

static void configure_system(GtkButton *button, gpointer user_data)
{
    int processes;
    int resources;
    char label_text[100];

    (void)button;
    (void)user_data;

    if (!read_integer(process_entry, &processes) ||
        !read_integer(resource_entry, &resources) ||
        processes < 1 || processes > MAX_PROCESSES ||
        resources < 1 || resources > MAX_RESOURCES) {
        show_message(
            "Invalid configuration.\n\n"
            "Enter a process count from 1 to 10 and a resource "
            "count from 1 to 10, then click Configure System again."
        );
        return;
    }

    clear_matrix_widgets();

    memset(&state, 0, sizeof(state));
    state.process_count = processes;
    state.resource_count = resources;

    gtk_box_append(
        GTK_BOX(matrix_box),
        make_section_label("Total instances of each resource")
    );

    for (int j = 0; j < resources; j++) {
        snprintf(label_text, sizeof(label_text), "Resource R%d total:", j);
        gtk_box_append(GTK_BOX(matrix_box), gtk_label_new(label_text));

        total_entry[j] = make_entry("3");
        gtk_widget_set_halign(total_entry[j], GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(matrix_box), total_entry[j]);
    }

    gtk_box_append(
        GTK_BOX(matrix_box),
        make_section_label("Allocation Matrix")
    );

    GtkWidget *allocation_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(allocation_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(allocation_grid), 8);

    gtk_grid_attach(
        GTK_GRID(allocation_grid),
        gtk_label_new("Process"),
        0, 0, 1, 1
    );

    for (int j = 0; j < resources; j++) {
        snprintf(label_text, sizeof(label_text), "R%d", j);
        gtk_grid_attach(
            GTK_GRID(allocation_grid),
            gtk_label_new(label_text),
            j + 1, 0, 1, 1
        );
    }

    for (int i = 0; i < processes; i++) {
        snprintf(label_text, sizeof(label_text), "P%d", i);

        gtk_grid_attach(
            GTK_GRID(allocation_grid),
            gtk_label_new(label_text),
            0, i + 1, 1, 1
        );

        for (int j = 0; j < resources; j++) {
            allocation_entry[i][j] = make_entry("0");

            gtk_grid_attach(
                GTK_GRID(allocation_grid),
                allocation_entry[i][j],
                j + 1, i + 1, 1, 1
            );
        }
    }

    gtk_box_append(GTK_BOX(matrix_box), allocation_grid);

    gtk_box_append(
        GTK_BOX(matrix_box),
        make_section_label("Maximum Demand Matrix")
    );

    GtkWidget *maximum_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(maximum_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(maximum_grid), 8);

    gtk_grid_attach(
        GTK_GRID(maximum_grid),
        gtk_label_new("Process"),
        0, 0, 1, 1
    );

    for (int j = 0; j < resources; j++) {
        snprintf(label_text, sizeof(label_text), "R%d", j);

        gtk_grid_attach(
            GTK_GRID(maximum_grid),
            gtk_label_new(label_text),
            j + 1, 0, 1, 1
        );
    }

    for (int i = 0; i < processes; i++) {
        snprintf(label_text, sizeof(label_text), "P%d", i);

        gtk_grid_attach(
            GTK_GRID(maximum_grid),
            gtk_label_new(label_text),
            0, i + 1, 1, 1
        );

        for (int j = 0; j < resources; j++) {
            maximum_entry[i][j] = make_entry("1");

            gtk_grid_attach(
                GTK_GRID(maximum_grid),
                maximum_entry[i][j],
                j + 1, i + 1, 1, 1
            );
        }
    }

    gtk_box_append(GTK_BOX(matrix_box), maximum_grid);

    gtk_box_append(
        GTK_BOX(matrix_box),
        make_section_label("Request Resources")
    );

    GtkWidget *request_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(request_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(request_grid), 8);

    gtk_box_append(
        GTK_BOX(matrix_box),
        request_grid
    );

    gtk_grid_attach(
        GTK_GRID(request_grid),
        gtk_label_new("Process ID (e.g. 0 for P0):"),
        0, 0, 1, 1
    );
    request_process_entry = make_entry("0");
    gtk_grid_attach(
        GTK_GRID(request_grid),
        request_process_entry,
        1, 0, 1, 1
    );

    for (int j = 0; j < resources; j++) {
        snprintf(label_text, sizeof(label_text), "R%d:", j);
        gtk_grid_attach(
            GTK_GRID(request_grid),
            gtk_label_new(label_text),
            j, 1, 1, 1
        );
        request_entry[j] = make_entry("0");
        gtk_grid_attach(
            GTK_GRID(request_grid),
            request_entry[j],
            j, 2, 1, 1
        );
    }

    GtkWidget *request_button =
        gtk_button_new_with_label("Submit Request");
    g_signal_connect(
        request_button,
        "clicked",
        G_CALLBACK(request_resources_clicked),
        NULL
    );
    gtk_grid_attach(
        GTK_GRID(request_grid),
        request_button,
        0, 3, resources, 1
    );

    configured = 1;
    data_applied = 0;

    show_message(
        "System configured successfully.\n\n"
        "Enter the total resources, allocation values and maximum "
        "demands above. Then click Apply Data before running an "
        "algorithm.\n\n"
        "Process IDs start at P0."
    );
}

static bool apply_data(void)
{
    int value;

    if (!configured) {
        show_message("Configure the system first.");
        return false;
    }

    memset(state.total, 0, sizeof(state.total));
    memset(state.available, 0, sizeof(state.available));
    memset(state.allocation, 0, sizeof(state.allocation));
    memset(state.maximum, 0, sizeof(state.maximum));
    memset(state.need, 0, sizeof(state.need));

    for (int j = 0; j < state.resource_count; j++) {
        if (!read_integer(total_entry[j], &value) || value < 1) {
            show_message(
                "Invalid resource total. Each resource must have "
                "at least one instance."
            );
            return false;
        }

        state.total[j] = value;
    }

    for (int i = 0; i < state.process_count; i++) {
        for (int j = 0; j < state.resource_count; j++) {
            if (!read_integer(allocation_entry[i][j], &value)) {
                show_message(
                    "Invalid allocation value. Enter non-negative "
                    "whole numbers in every allocation field."
                );
                return false;
            }

            state.allocation[i][j] = value;

            if (!read_integer(maximum_entry[i][j], &value)) {
                show_message(
                    "Invalid maximum demand. Enter non-negative "
                    "whole numbers in every maximum field."
                );
                return false;
            }

            state.maximum[i][j] = value;
        }
    }

    if (!calculate_need(&state)) {
        show_message(
            "The resource data is invalid.\n\n"
            "Check that allocation does not exceed maximum demand "
            "and that allocated resources do not exceed total resources."
        );
        return false;
    }

    data_applied = 1;

    show_message(
        "Data applied successfully.\n\n"
        "The system data is valid. You can now calculate Need and "
        "Available, check the safe sequence, or check whether all "
        "processes can finish."
    );

    return true;
}

static void show_resource_graph_clicked(
    GtkButton *button,
    gpointer user_data
)
{
    GtkWidget *window;
    GtkWidget *layout;
    GtkWidget *drawing_area;
    GtkWidget *scroller;
    GtkWidget *note;
    int graph_height;

    (void)button;
    (void)user_data;

    if (!data_applied && !apply_data()) {
        return;
    }

    window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Resource Allocation Graph");
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 620);
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(main_window));

    layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(layout, 12);
    gtk_widget_set_margin_bottom(layout, 12);
    gtk_widget_set_margin_start(layout, 12);
    gtk_widget_set_margin_end(layout, 12);

    note = gtk_label_new(
        "Allocation arrows point from a resource to a process. Request "
        "arrows point from a process to a resource. Resource labels show "
        "available / total instances."
    );
    gtk_label_set_wrap(GTK_LABEL(note), true);
    gtk_widget_set_halign(note, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(layout), note);

    scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroller),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );

    drawing_area = gtk_drawing_area_new();
    graph_height = (state.process_count > state.resource_count
        ? state.process_count
        : state.resource_count) * 58 + 130;
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), 820);
    gtk_drawing_area_set_content_height(
        GTK_DRAWING_AREA(drawing_area),
        graph_height
    );
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(drawing_area),
        draw_resource_graph,
        NULL,
        NULL
    );
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroller),
        drawing_area
    );
    gtk_widget_set_vexpand(scroller, true);
    gtk_box_append(GTK_BOX(layout), scroller);

    gtk_window_set_child(GTK_WINDOW(window), layout);
    gtk_window_present(GTK_WINDOW(window));
}

static void request_resources_clicked(
    GtkButton *button,
    gpointer user_data
)
{
    int process_id;
    int request[MAX_RESOURCES] = {0};
    char output[2048];
    size_t used = 0;

    (void)button;
    (void)user_data;

    if (!configured) {
        show_message("Configure the system before submitting a request.");
        return;
    }

    if (!data_applied && !apply_data()) {
        return;
    }

    if (!read_integer(request_process_entry, &process_id) ||
        process_id >= state.process_count) {
        show_message(
            "Invalid process ID.\n\n"
            "Enter a process number from 0 to the configured process count "
            "minus one."
        );
        return;
    }

    for (int j = 0; j < state.resource_count; j++) {
        if (!read_integer(request_entry[j], &request[j])) {
            show_message(
                "Invalid resource request.\n\n"
                "Enter a non-negative whole number for every resource."
            );
            return;
        }
    }

    if (!request_resources(&state, process_id, request)) {
        show_message(
            "Request denied.\n\n"
            "The request is invalid, exceeds the process's remaining need "
            "or available resources, or would leave the system unsafe. "
            "No resources were allocated."
        );
        return;
    }

    for (int j = 0; j < state.resource_count; j++) {
        char value[32];
        snprintf(value, sizeof(value), "%d", state.allocation[process_id][j]);
        gtk_editable_set_text(
            GTK_EDITABLE(allocation_entry[process_id][j]),
            value
        );
    }

    used += (size_t)snprintf(
        output + used,
        sizeof(output) - used,
        "REQUEST GRANTED\n\n"
        "P%d's request was granted and the system remains safe.\n"
        "Updated available resources:\n",
        process_id
    );

    for (int j = 0; j < state.resource_count; j++) {
        used += (size_t)snprintf(
            output + used,
            sizeof(output) - used,
            "R%d: %d%s",
            j,
            state.available[j],
            j == state.resource_count - 1 ? "\n" : "   "
        );
    }

    show_message(output);
}

static void apply_data_clicked(
    GtkButton *button,
    gpointer user_data
)
{
    (void)button;
    (void)user_data;

    apply_data();
}

static void show_need_clicked(
    GtkButton *button,
    gpointer user_data
)
{
    char output[8192];
    size_t used = 0;

    (void)button;
    (void)user_data;

    if (!data_applied && !apply_data()) {
        return;
    }

    used += (size_t)snprintf(
        output + used,
        sizeof(output) - used,
        "NEED AND AVAILABLE\n\n"
        "Available resources:\n"
    );

    for (int j = 0; j < state.resource_count; j++) {
        used += (size_t)snprintf(
            output + used,
            sizeof(output) - used,
            "R%d: %d%s",
            j,
            state.available[j],
            j == state.resource_count - 1 ? "\n\n" : "   "
        );
    }

    used += (size_t)snprintf(
        output + used,
        sizeof(output) - used,
        "Need Matrix:\n       "
    );

    for (int j = 0; j < state.resource_count; j++) {
        used += (size_t)snprintf(
            output + used,
            sizeof(output) - used,
            "R%d  ",
            j
        );
    }

    used += (size_t)snprintf(
        output + used,
        sizeof(output) - used,
        "\n"
    );

    for (int i = 0; i < state.process_count; i++) {
        used += (size_t)snprintf(
            output + used,
            sizeof(output) - used,
            "P%d     ",
            i
        );

        for (int j = 0; j < state.resource_count; j++) {
            used += (size_t)snprintf(
                output + used,
                sizeof(output) - used,
                "%d   ",
                state.need[i][j]
            );
        }

        used += (size_t)snprintf(
            output + used,
            sizeof(output) - used,
            "\n"
        );
    }

    show_message(output);
}

static void banker_clicked(
    GtkButton *button,
    gpointer user_data
)
{
    int safe_sequence[MAX_PROCESSES];
    char output[4096];
    size_t used = 0;

    (void)button;
    (void)user_data;

    if (!data_applied && !apply_data()) {
        return;
    }

    if (check_safe_state(&state, safe_sequence)) {
        used += (size_t)snprintf(
            output + used,
            sizeof(output) - used,
            "BANKER'S ALGORITHM\n\n"
            "The system is in a SAFE state.\n\n"
            "Safe sequence:\n"
        );

        for (int i = 0; i < state.process_count; i++) {
            used += (size_t)snprintf(
                output + used,
                sizeof(output) - used,
                "P%d%s",
                safe_sequence[i],
                i == state.process_count - 1 ? "\n" : " -> "
            );
        }
    } else {
        snprintf(
            output,
            sizeof(output),
            "BANKER'S ALGORITHM\n\n"
            "The system is in an UNSAFE state.\n"
            "No safe sequence was found for the current data."
        );
    }

    show_message(output);
}

static void deadlock_clicked(
    GtkButton *button,
    gpointer user_data
)
{
    bool deadlocked[MAX_PROCESSES] = { false };
    int count;
    char output[4096];
    size_t used = 0;

    (void)button;
    (void)user_data;

    if (!data_applied && !apply_data()) {
        return;
    }

    count = detect_deadlock(&state, deadlocked);

    if (count < 0) {
        show_message(
            "COMPLETION FEASIBILITY CHECK\n\n"
            "The resource state is invalid, so the check could not run."
        );
        return;
    }

    if (count == 0) {
        show_message(
            "COMPLETION FEASIBILITY CHECK\n\n"
            "All processes can finish in a safe order. This does not "
            "test for runtime deadlock."
        );
        return;
    }

    used += (size_t)snprintf(
        output + used,
        sizeof(output) - used,
        "COMPLETION FEASIBILITY CHECK\n\n"
        "Processes that cannot be shown to finish: %d\n\n"
        "This indicates an unsafe state, not necessarily a runtime "
        "deadlock.\n\n",
        count
    );

    for (int i = 0; i < state.process_count; i++) {
        if (deadlocked[i]) {
            used += (size_t)snprintf(
                output + used,
                sizeof(output) - used,
                "P%d cannot currently be shown to finish.\n",
                i
            );
        }
    }

    show_message(output);
}

static void recovery_clicked(
    GtkButton *button,
    gpointer user_data
)
{
    int process_id;
    char output[1024];

    (void)button;
    (void)user_data;

    if (!data_applied && !apply_data()) {
        return;
    }

    if (!read_integer(recovery_entry, &process_id) ||
        process_id < 0 ||
        process_id >= state.process_count) {
        show_message(
            "Invalid process ID.\n\n"
            "Enter a valid process number, for example 0 for P0."
        );
        return;
    }

    if (recover_from_deadlock(&state, process_id)) {
        data_applied = 1;

        snprintf(
            output,
            sizeof(output),
            "RECOVERY SUCCESSFUL\n\n"
            "Resources held by P%d have been released by the simulator.\n"
            "Available resources and Need values have been recalculated.\n\n"
            "This is a simulated recovery from an unsafe state, not "
            "termination of the process.\n\n"
            "Click 'Need / Available' or 'Check Safe State' "
            "to inspect the updated system.",
            process_id
        );

        show_message(output);
    } else {
        show_message(
            "Recovery could not be completed.\n\n"
            "Check that the selected process holds allocated resources "
            "and that the system data is valid."
        );
    }
}

void gui_activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *heading;
    GtkWidget *description;
    GtkWidget *configuration_box;
    GtkWidget *button_box;
    GtkWidget *output_scroller;
    GtkWidget *output_label;
    GtkWidget *button;
    GtkWidget *recovery_box;

    (void)user_data;

    window = gtk_application_window_new(app);
    main_window = window;
    gtk_window_set_title(
        GTK_WINDOW(window),
        "Deadlock Simulation System"
    );
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 750);

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);

    heading = gtk_label_new("Interactive Deadlock Simulation System");
    gtk_widget_add_css_class(heading, "title-1");
    gtk_widget_set_halign(heading, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(main_box), heading);

    description = gtk_label_new(
        "Configure processes and resources, calculate Need and Available, "
        "check whether all processes can finish, and simulate resource "
        "release from an unsafe state."
    );
    gtk_label_set_wrap(GTK_LABEL(description), true);
    gtk_widget_set_halign(description, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(main_box), description);

    configuration_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    gtk_box_append(
        GTK_BOX(configuration_box),
        gtk_label_new("Number of processes (1-10):")
    );

    process_entry = make_entry("3");
    gtk_box_append(GTK_BOX(configuration_box), process_entry);

    gtk_box_append(
        GTK_BOX(configuration_box),
        gtk_label_new("Number of resources (1-10):")
    );

    resource_entry = make_entry("3");
    gtk_box_append(GTK_BOX(configuration_box), resource_entry);

    button = gtk_button_new_with_label("Configure System");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(configure_system),
        NULL
    );
    gtk_box_append(GTK_BOX(configuration_box), button);

    gtk_box_append(GTK_BOX(main_box), configuration_box);

    matrix_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    GtkWidget *matrix_scroller = gtk_scrolled_window_new();
    gtk_widget_set_size_request(matrix_scroller, -1, 300);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(matrix_scroller),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(matrix_scroller),
        matrix_box
    );

    gtk_box_append(GTK_BOX(main_box), matrix_scroller);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    button = gtk_button_new_with_label("Apply Data");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(apply_data_clicked),
        NULL
    );
    gtk_box_append(GTK_BOX(button_box), button);

    button = gtk_button_new_with_label("Need / Available");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(show_need_clicked),
        NULL
    );
    gtk_box_append(GTK_BOX(button_box), button);

    button = gtk_button_new_with_label("Check Safe State");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(banker_clicked),
        NULL
    );
    gtk_box_append(GTK_BOX(button_box), button);

    button = gtk_button_new_with_label("Check Completion Feasibility");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(deadlock_clicked),
        NULL
    );
    gtk_box_append(GTK_BOX(button_box), button);

    button = gtk_button_new_with_label("Show Resource Allocation Graph");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(show_resource_graph_clicked),
        NULL
    );
    gtk_box_append(GTK_BOX(button_box), button);

    gtk_box_append(GTK_BOX(main_box), button_box);

    recovery_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    gtk_box_append(
        GTK_BOX(recovery_box),
        gtk_label_new("Process ID to release resources from (e.g. 0 for P0):")
    );

    recovery_entry = make_entry("0");
    gtk_box_append(GTK_BOX(recovery_box), recovery_entry);

    button = gtk_button_new_with_label("Recover Process");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(recovery_clicked),
        NULL
    );
    gtk_box_append(GTK_BOX(recovery_box), button);

    gtk_box_append(GTK_BOX(main_box), recovery_box);

    output_label = make_section_label("Simulation Output");
    gtk_box_append(GTK_BOX(main_box), output_label);

    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), false);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), false);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_WORD_CHAR);

    output_scroller = gtk_scrolled_window_new();
    gtk_widget_set_size_request(output_scroller, -1, 150);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(output_scroller),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(output_scroller),
        output_view
    );

    gtk_box_append(GTK_BOX(main_box), output_scroller);

    gtk_window_set_child(GTK_WINDOW(window), main_box);

    show_message(
        "Welcome!\n\n"
        "1. Choose the number of processes and resources.\n"
        "2. Click Configure System.\n"
        "3. Enter resource totals, allocations and maximum demands.\n"
        "4. Click Apply Data.\n"
        "5. Enter a process ID and resource request, then click "
        "Submit Request to ask the Banker's Algorithm to grant or deny it.\n"
        "6. Run the simulation buttons to inspect the results.\n\n"
        "Process IDs start at P0."
    );

    gtk_window_present(GTK_WINDOW(window));
}
