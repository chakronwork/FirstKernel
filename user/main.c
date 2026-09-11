#include "ulib.h"

int main(void)
{
    char buffer[64];

    print("=== FirstOS User Shell (Ring 3 C) ===\n");

    for (;;) {
        print("shell> ");

        int len = readline(buffer, sizeof(buffer));
        if (len <= 0) {
            continue;
        }

        if (strcmp(buffer, "help") == 0) {
            print("Available commands:\n");
            print("  help  - show this message\n");
            print("  clear - clear the screen\n");
        } else if (strcmp(buffer, "clear") == 0) {
            for (int i = 0; i < 25; i++) {
                print("\n");
            }
        } else {
            print("Unknown command. Type help for commands list.\n");
        }
    }

    return 0;
}
