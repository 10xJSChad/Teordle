#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define clear_screen() system("clear")
#endif

#if defined(_WIN32) || defined(_WIN64)
#define clear_screen() system("cls")
#endif

#define COLOR_GREEN "\x1B[32m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_WHITE "\x1B[37m"

#define LETTER_SEPERATOR_STR "_ "
#define NULL_CHAR '\0'
#define NEWLINE_STR "\n"
#define NEWLINE_CHAR (char) '\n'

#define WORD_LENGTH 5
#define MAX_GUESSES 6
#define LAST_CHARACTER WORD_LENGTH
#define GUESS_ARRAY_SIZE MAX_GUESSES * WORD_LENGTH

#define NOT_IN_WORD 0
#define IN_WORD_WRONG_INDEX 1
#define IN_WORD_CORRECT_INDEX 2

#define PLAYER_WON true
#define PLAYER_LOST false

#define assert_print(condition, string) \
    if (!(condition)) {                 \
        printf("%s", string);           \
        terminate(1);                   \
    }


void get_input();
void terminate(uint8_t exit_code);

FILE* file_ptr;

size_t words_length = 0;
size_t current_line = 0;

unsigned char user_input[WORD_LENGTH + 1];
unsigned char current_word[WORD_LENGTH + 1];
unsigned char guess_list[GUESS_ARRAY_SIZE];

bool game_over = false;


uint8_t
check_letter_in_word(
    unsigned char letter,
    size_t guess_list_index)
{
    if (current_word[guess_list_index] == letter)
        return IN_WORD_CORRECT_INDEX;

    for (uint8_t i = 0; i < WORD_LENGTH; ++i) 
        if (current_word[i] == letter)
            return IN_WORD_WRONG_INDEX;

    return NOT_IN_WORD;

}


unsigned char*
determine_letter_color(
    unsigned char letter,
    size_t guess_list_index)
{
    switch (check_letter_in_word(
        letter, guess_list_index))
    {
    case IN_WORD_CORRECT_INDEX:
        return COLOR_GREEN;
    case IN_WORD_WRONG_INDEX:
        return COLOR_YELLOW;
    default:
        return COLOR_WHITE;  
    }
}


void
print_letter(
    unsigned char letter,
    char* color)
{
    printf("%s%c%s ", color, letter, COLOR_WHITE);
}


void
assign_random_word()
{
start_label:
    unsigned char ch;

    if (!file_ptr) {
        file_ptr = fopen("words.txt" , "r");

        // prints a message and exits if file_ptr is NULL;
        assert_print(file_ptr, "Unable to find or open words.txt");

        // get length of word list
        fseek(file_ptr, 0, SEEK_END);
        words_length = ftell(file_ptr) - WORD_LENGTH - 1; // subtract to adjust for the last word
    }

    // really should use a better random function, barely ever see
    // any of the words in the last half of the word list
    fseek(file_ptr, rand() % words_length, SEEK_SET);

    // get a word from the list
    while (ch = fgetc(file_ptr)) {
        if (ch == NEWLINE_CHAR) {
            for (size_t i = 0; i < WORD_LENGTH; ++i)
                current_word[i] = toupper(fgetc(file_ptr));
            return;
        }
    }

    // EOF reached, try again
    goto start_label;
}


void
string_toupper(
    unsigned char* str)
{
    while (*str)
        *(str++) = toupper(*str);
}


bool
is_input_valid(
    unsigned char* str)
{
    // check the validity of the first 5
    for (int i = 0; i < WORD_LENGTH; ++i)
        if (!str[i] || !isalpha(str[i])) // yay ascii only!
            return false;

    // forcing NULL at input[5] just made things annoying
    // when you accidentally type one letter too many
    // or your brain fails to tell you that a certain word has
    // six letters and not five, and it gets treated as a
    // valid input because it was terminated after five letters.
    return !str[LAST_CHARACTER];
}


void
render()
{
    clear_screen();
    
    unsigned char* color;

    for (size_t i = 0; i < GUESS_ARRAY_SIZE; ++i) {
        size_t letter_guess_array_index = i % WORD_LENGTH;

        if (letter_guess_array_index == 0 && i != 0)
            printf(NEWLINE_STR);

        if (guess_list[i])
            print_letter(
                guess_list[i], 
                determine_letter_color(guess_list[i], letter_guess_array_index));
        else
            printf(LETTER_SEPERATOR_STR);
    }

    printf(NEWLINE_STR);
}


void 
end_game(
    bool victory)
{
    game_over = true;

    if (victory)
        printf("\nYou win!\n\n");
    else
        printf("\nYou lose, the word was: %s\n\n", current_word);

    printf("PLAY | EXIT\n");
}


void
check_victory()
{

/*
 * I'm not into the extra indentation level for a while (true) loop
 * so we're getting jiggy with the goto up in here.
 */

start_label:
    bool guessed_correct_word = strncmp(
        user_input, current_word, WORD_LENGTH) == 0;

    render();

    if (guessed_correct_word)
        end_game(PLAYER_WON);
    if (current_line == MAX_GUESSES && !guessed_correct_word)
        end_game(PLAYER_LOST);

    get_input();
    goto start_label;
}


void
clear_guess_list()
{
    for (size_t i = 0; i < GUESS_ARRAY_SIZE; ++i)
        guess_list[i] = NULL_CHAR;
}


int
start_game()
{
    // current attempt number
    current_line = 0;
    game_over = false;

    clear_screen();
    clear_guess_list();
    assign_random_word();
    check_victory();
}


void
get_input()
{
    unsigned char guess[7];

    if (fgets(guess , WORD_LENGTH + 2 , stdin)) {
        guess[strcspn(guess, NEWLINE_STR)] = NULL_CHAR;
        string_toupper(guess);
    }

    if (strlen(guess) > WORD_LENGTH) 
        get_input();

    if (strcmp(guess , "PLAY") == 0) start_game();
    if (strcmp(guess , "EXIT") == 0) terminate(0);

    if (is_input_valid(guess) && !game_over) {
        for (int i = 0; i < WORD_LENGTH; ++i) {
            user_input[i] = guess[i];
            guess_list[(5 * current_line) + i] = guess[i];
        }
        ++current_line;
    }
}


int
main()
{
    srand(time(NULL));
    
    printf("Teordle - Wordle, but in the terminal\n");
    printf("---------------------------------------\n");
    printf("PLAY | EXIT\n");
    get_input();
    clear_screen();

    terminate(0);
}



void 
terminate(
    uint8_t exit_code)
{
    if (file_ptr)
        fclose(file_ptr);

    exit(exit_code);
}
