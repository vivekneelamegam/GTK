#include <gtk/gtk.h>
#include <string.h>
#include <mysql/mysql.h>

GtkWidget *entry;
GtkWidget *listbox;
MYSQL *conn;

void add_task(GtkWidget *widget, gpointer data) {
    const char *task = gtk_entry_get_text(GTK_ENTRY(entry));

    if (strlen(task) == 0) {
        return;
    }

    // Insert task into MySQL database
    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO tasks (description) VALUES ('%s')", task);
    if (mysql_query(conn, query)) {
        g_print("Failed to insert task: %s\n", mysql_error(conn));
        return;
    }

    // Add task to the listbox
    GtkWidget *label = gtk_label_new(task);
    GtkWidget *row = gtk_list_box_row_new();
    gtk_container_add(GTK_CONTAINER(row), label);
    gtk_container_add(GTK_CONTAINER(listbox), row);
    gtk_widget_show_all(listbox);

    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void remove_task(GtkWidget *widget, gpointer data) {
    GtkListBoxRow *row = gtk_list_box_get_selected_row(GTK_LIST_BOX(listbox));
    if (row != NULL) {
        // Remove task from MySQL database
        GtkWidget *label = gtk_bin_get_child(GTK_BIN(row));
        const char *task = gtk_label_get_text(GTK_LABEL(label));
        
        char query[512];
        snprintf(query, sizeof(query), "DELETE FROM tasks WHERE description='%s'", task);
        if (mysql_query(conn, query)) {
            g_print("Failed to remove task: %s\n", mysql_error(conn));
            return;
        }

        // Remove task from the listbox
        gtk_widget_destroy(GTK_WIDGET(row));
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *add_button;
    GtkWidget *remove_button;
    GtkWidget *scroll;

    gtk_init(&argc, &argv);

    // Initialize MySQL connection
    conn = mysql_init(NULL);
    if (conn == NULL) {
        g_print("mysql_init() failed\n");
        return 1;
    }

    if (mysql_real_connect(conn, "localhost", "root", "1471", "todo", 0, NULL, 0) == NULL) {
        g_print("mysql_real_connect() failed\n");
        mysql_close(conn);
        return 1;
    }

    // Create tasks table if it doesn't exist
    if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS tasks (id INT AUTO_INCREMENT PRIMARY KEY, description TEXT NOT NULL)")) {
        g_print("Failed to create table: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "To-Do List");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box to hold the widgets
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Entry widget for task input
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    // Horizontal box for buttons
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Add Task button
    add_button = gtk_button_new_with_label("Add Task");
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_task), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), add_button, TRUE, TRUE, 0);

    // Remove Task button
    remove_button = gtk_button_new_with_label("Remove Task");
    g_signal_connect(remove_button, "clicked", G_CALLBACK(remove_task), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), remove_button, TRUE, TRUE, 0);

    // ListBox to display tasks
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scroll), listbox);

    gtk_widget_show_all(window);

    gtk_main();

    // Close MySQL connection
    mysql_close(conn);

    return 0;
}
