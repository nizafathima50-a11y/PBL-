#include "gui.h"

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
static GtkWidget *matrix_box;
static GtkWidget *output_view;

static int configured = 0;
static int data_applied = 0;

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
        "Available, check the safe sequence, or detect deadlock."
    );

    return true;
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

    if (count <= 0) {
        show_message(
            "DEADLOCK DETECTION\n\n"
            "No deadlocked processes were detected."
        );
        return;
    }

    used += (size_t)snprintf(
        output + used,
        sizeof(output) - used,
        "DEADLOCK DETECTION\n\n"
        "Deadlocked processes detected: %d\n\n",
        count
    );

    for (int i = 0; i < state.process_count; i++) {
        if (deadlocked[i]) {
            used += (size_t)snprintf(
                output + used,
                sizeof(output) - used,
                "P%d is deadlocked.\n",
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
            "Resources held by P%d have been released.\n"
            "Available resources and Need values have been recalculated.\n\n"
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
        "check system safety, detect deadlocks, and recover resources."
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

    button = gtk_button_new_with_label("Detect Deadlock");
    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(deadlock_clicked),
        NULL
    );
    gtk_box_append(GTK_BOX(button_box), button);

    gtk_box_append(GTK_BOX(main_box), button_box);

    recovery_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    gtk_box_append(
        GTK_BOX(recovery_box),
        gtk_label_new("Process ID to recover (e.g. 0 for P0):")
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
        "5. Run the simulation buttons to inspect the results.\n\n"
        "Process IDs start at P0."
    );

    gtk_window_present(GTK_WINDOW(window));
}
