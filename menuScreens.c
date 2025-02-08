// Function to Display the Menu
void sysgui_draw_menu()
{
    for (int i = 0; sysgui_menu_list[i].label != NULL; i++)
    {
        // Print Menu Item Label
        printf("%s: ", sysgui_menu_list[i].label);

        // Print Options
        const char **opt = sysgui_menu_list[i].options;
        while (*opt)
        {
            printf("[%s] ", *opt);
            opt++;
        }
        printf("\n");
    }
}

// Function to Handle User Selection (Placeholder)
void sysgui_handle_selection(uint8_t index)
{
    if (sysgui_menu_list[index].label == NULL)
    {
        printf("Invalid selection.\n");
        return;
    }
    printf("You selected: %s\n", sysgui_menu_list[index].label);
}

// Main Function
// int main() {
//     sysgui_draw_menu();

//     // Example user selection
//     uint8_t user_selection = 2; // Select CONTRAST
//     sysgui_handle_selection(user_selection);

//     return 0;
// }
