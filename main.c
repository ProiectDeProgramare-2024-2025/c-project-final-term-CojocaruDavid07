#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILE_NAME "wallet.dat"

#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_CYAN    "\033[0;36m"

typedef struct {
    int id;
    char number[20];
    char holder[100];
    char expiration[10];
    float balance;
    int usageCount;
} Card;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    usleep(100000);
    system("clear");
#endif
}

void prepopulateFile() {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        file = fopen(FILE_NAME, "w");
        if (file == NULL) {
            printf(COLOR_RED "Error creating file!\n" COLOR_RESET);
            return;
        }
        Card card1 = {1, "1234567890123456", "John Doe", "12/25", 100.00, 0};
        Card card2 = {2, "9876543210987654", "Jane Smith", "05/27", 250.00, 0};
        
        fwrite(&card1, sizeof(Card), 1, file);
        fwrite(&card2, sizeof(Card), 1, file);
        fclose(file);
    } else {
        fclose(file);
    }
}

int compareCards(const void *a, const void *b) {
    Card *cardA = (Card *)a;
    Card *cardB = (Card *)b;
    return cardB->usageCount - cardA->usageCount;
}

void viewCards() {
    clearScreen();
    printf(COLOR_CYAN "===== View Existing Cards =====\n\n" COLOR_RESET);

    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        printf(COLOR_RED "Error opening file!\n" COLOR_RESET);
        return;
    }

    Card cards[100];
    int count = 0;
    while (fread(&cards[count], sizeof(Card), 1, file)) {
        count++;
    }
    fclose(file);

    qsort(cards, count, sizeof(Card), compareCards);

    for (int i = 0; i < count; i++) {
        printf("ID: %d\n", cards[i].id);
        printf("Card Number: %s\n", cards[i].number);
        printf("Cardholder: %s\n", cards[i].holder);
        printf("Expiration: %s\n", cards[i].expiration);
        printf("Balance: %.2f\n", cards[i].balance);
        printf("Usage Count: %d\n", cards[i].usageCount);
        printf("---\n");
    }

    printf(COLOR_YELLOW "\nPress Enter to return to the main menu..." COLOR_RESET);
    getchar();
    getchar();
}

void addCard() {
    clearScreen();
    Card newCard;
    printf(COLOR_CYAN "===== Add New Card =====\n" COLOR_RESET);
    printf("Enter card number (16 digits): ");
    scanf("%s", newCard.number);

    if (strlen(newCard.number) != 16) {
        printf(COLOR_RED "Invalid card number length. Must be 16 digits.\n" COLOR_RESET);
        printf(COLOR_YELLOW "Press Enter to return to the main menu..." COLOR_RESET);
        getchar(); getchar();
        return;
    }

    printf("Enter cardholder name (First Last): ");
    getchar();
    fgets(newCard.holder, sizeof(newCard.holder), stdin);
    newCard.holder[strcspn(newCard.holder, "\n")] = 0;

    printf("Enter expiration date (MM/YY): ");
    scanf("%s", newCard.expiration);

    printf("Enter initial balance: ");
    scanf("%f", &newCard.balance);

    newCard.usageCount = 0;

    FILE *file = fopen(FILE_NAME, "a");
    if (file == NULL) {
        printf(COLOR_RED "Error opening file for writing!\n" COLOR_RESET);
        return;
    }

    int lastId = 0;
    FILE *checkFile = fopen(FILE_NAME, "r");
    if (checkFile != NULL) {
        Card lastCard;
        while (fread(&lastCard, sizeof(Card), 1, checkFile)) {
            lastId = lastCard.id;
        }
        fclose(checkFile);
    }
    newCard.id = lastId + 1;

    fwrite(&newCard, sizeof(Card), 1, file);
    fclose(file);

    printf(COLOR_GREEN "\nCard added successfully!\n" COLOR_RESET);
    printf(COLOR_YELLOW "Press Enter to return to the main menu..." COLOR_RESET);
    getchar(); getchar();
}

void deleteCard() {
    clearScreen();
    int targetId;
    printf(COLOR_CYAN "===== Delete Card =====\n" COLOR_RESET);
    printf("Enter card ID to delete: ");
    scanf("%d", &targetId);

    FILE *file = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.dat", "w");
    if (file == NULL || temp == NULL) {
        printf(COLOR_RED "Error opening files!\n" COLOR_RESET);
        return;
    }

    Card card;
    int found = 0;
    while (fread(&card, sizeof(Card), 1, file)) {
        if (card.id == targetId) {
            found = 1;
        } else {
            fwrite(&card, sizeof(Card), 1, temp);
        }
    }

    fclose(file);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.dat", FILE_NAME);

    if (found) {
        printf(COLOR_GREEN "\nCard deleted if it existed.\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "Card with ID %d not found.\n" COLOR_RESET, targetId);
    }

    printf(COLOR_YELLOW "Press Enter to return to the main menu..." COLOR_RESET);
    getchar(); getchar();
}

void updateBalance(int operation) {
    clearScreen();
    int targetId;
    float amount;
    printf(COLOR_CYAN "===== %s Money =====\n" COLOR_RESET, operation == 1 ? "Add" : "Withdraw");
    printf("Enter card ID: ");
    scanf("%d", &targetId);

    printf("Enter amount: ");
    scanf("%f", &amount);

    FILE *file = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.dat", "w");
    if (file == NULL || temp == NULL) {
        printf(COLOR_RED "Error opening files!\n" COLOR_RESET);
        return;
    }

    Card card;
    int found = 0;
    while (fread(&card, sizeof(Card), 1, file)) {
        if (card.id == targetId) {
            found = 1;
            if (operation == 0 && amount > card.balance) {
                printf(COLOR_RED "Insufficient funds.\n" COLOR_RESET);
                fclose(file);
                fclose(temp);
                remove("temp.dat");
                printf(COLOR_YELLOW "Press Enter to return to the main menu..." COLOR_RESET);
                getchar(); getchar();
                return;
            }
            card.balance += operation ? amount : -amount;
            card.usageCount++;
        }
        fwrite(&card, sizeof(Card), 1, temp);
    }

    fclose(file);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.dat", FILE_NAME);

    if (found) {
        printf(COLOR_GREEN "\nBalance updated successfully.\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "Card not found.\n" COLOR_RESET);
    }

    printf(COLOR_YELLOW "Press Enter to return to the main menu..." COLOR_RESET);
    getchar(); getchar();
}

void displayMainMenu() {
    clearScreen();
    printf(COLOR_CYAN "===== Electronic Wallet Menu =====\n" COLOR_RESET);
    printf("1. Add New Card\n");
    printf("2. View Existing Cards\n");
    printf("3. Delete Card\n");
    printf("4. Add Money\n");
    printf("5. Withdraw Money\n");
    printf("6. Exit\n");
    printf("Choose an option: ");
}

int main() {
    int choice;
    prepopulateFile();

    while (1) {
        displayMainMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addCard();
                break;
            case 2:
                viewCards();
                break;
            case 3:
                deleteCard();
                break;
            case 4:
                updateBalance(1);
                break;
            case 5:
                updateBalance(0);
                break;
            case 6:
                printf(COLOR_GREEN "Exiting program. Goodbye!\n" COLOR_RESET);
                return 0;
            default:
                printf(COLOR_RED "Invalid choice. Please try again.\n" COLOR_RESET);
                getchar(); getchar();
        }
    }

    return 0;
}
