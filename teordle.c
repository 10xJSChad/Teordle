#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLOR_GREEN "\x1B[32m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_WHITE "\x1B[37m"
#define NULL_CHAR '\0'

#define assert_print(condition, string) \
    if (!(condition)) {                 \
        printf("%s", string);           \
        exit(1);                        \
    }


void get_input();

FILE* file_ptr;

int words_length = 0;
int current_line = 0;

char current_word[6];
char user_input[6];
char guess_list[30];

bool game_over = false;


bool
contains_character(
    char ch)
{
    for (int i = 0; i < 5; ++i) 
        if (current_word[i] == ch)
            return(true);

    return(false);

}


void
print_char(
    char ch ,
    char* color)
{
    printf("%s%c%s " , color, ch, COLOR_WHITE);
}


void
get_word()
{
    char ch;

    if (!file_ptr) {
        file_ptr = fopen("words.txt" , "r");

        // prints a message and exits if file_ptr is NULL;
        assert_print(file_ptr, "Unable to find or open words.txt");

        // Get length of word list
        fseek(file_ptr, 0, SEEK_END);
        words_length = ftell(file_ptr) - 6; // subtract to adjust for the last word
    }

    // really should use a better random function, barely ever see
    // any of the words in the last half of the word list
    fseek(file_ptr, rand() % words_length, SEEK_SET);

    // Get a word from the list
    while (ch = fgetc(file_ptr)) {
        if (ch == '\n') {
            for (int i = 0; i < 5; ++i)
                current_word[i] = toupper(fgetc(file_ptr));
            return;
        }
    }

    // EOF reached, try again
    get_word();
}


void
clear()
{
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    system("clear");
    #endif

    #if defined(_WIN32) || defined(_WIN64)
    system("cls");
    #endif
}


void
string_toupper(
    char* str)
{
    int i = 0;
    while (str[i]) {
        str[i] = toupper(str[i]);
        ++i;
    }
}


bool
is_input_valid(
    char* str)
{
    for (int i = 0; i < 5; ++i) {
        if (!str[i])
            return(false);
    }

    return(!str[5]);
}


void
render()
{
    clear();
    int curr = 0;
    for (int i = 0; i < 30; ++i) {
        if (!guess_list[i])
            printf("_ ");
        else {
            if (guess_list[i] == current_word[curr]) {
                print_char(guess_list[i] , COLOR_GREEN);
            } else {
                if (contains_character(guess_list[i])) {
                    print_char(guess_list[i] , COLOR_YELLOW);
                } else
                    print_char(guess_list[i] , COLOR_WHITE);
            }
        }

        if ((i + 1) % 5 == 0)
            printf("\n");

        ++curr;
        if (curr >= 5)
            curr = 0;
    }
}


void
check_victory()
{
    bool guessed_right = strcmp(user_input , current_word) == 0;
    render();

    if (current_line > 5 && !guessed_right) {
        printf("\nYou lose, the word was: %s \n\n" , current_word);
        printf("PLAY | EXIT\n");
        get_input();
    }

    if (guessed_right) {
        printf("\nYou win!\n\n");
        printf("PLAY | EXIT\n");
        get_input();
    } else {
        get_input();
        check_victory();
    }
}


void
clear_guess_list()
{
    for (int i = 0; i < 30; ++i)
        guess_list[i] = NULL_CHAR;
}


int
start_game()
{
    // Current attemps
    current_line = 0;

    clear();
    clear_guess_list();

    get_word();
    check_victory();

    return(0);
}


void
get_input()
{
    char str[7];

    if (fgets(str , sizeof(str) , stdin)) {
        str[strcspn(str , "\n")] = '\0';
        string_toupper(str);
    }

    if (strlen(str) > 5) 
        get_input();

    if (strcmp(str , "PLAY") == 0) start_game();
    if (strcmp(str , "EXIT") == 0) exit(0);

    if (is_input_valid(str)) {
        for (int i = 0; i < 5; ++i) {
            user_input[i] = str[i];
            guess_list[(5 * current_line) + i] = str[i];
        }
        ++current_line;
    }
}


int
main()
{
    srand(time(NULL));
    
    while (true) {
        printf("Teordle - Wordle, but in the terminal\n");
        printf("---------------------------------------\n");
        printf("PLAY | EXIT\n");
        get_input();
        clear();
    }

    return(0);
}
