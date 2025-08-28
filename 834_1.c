static int count_directives_sub(const char *directive, ap_directive_t *current) {
    int count = 0;
    while (current != NULL) {
        if (current->first_child != NULL)
            count += count_directives_sub(directive, current->first_child);
        if (strcasecmp(current->directive, directive) == 0)
            count++;
        current = current->next;
    }
    return count;
}