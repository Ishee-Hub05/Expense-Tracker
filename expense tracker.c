#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "expenses.csv"

typedef struct Expense {
    int id;
    char date[20];
    double amount;
    char category[50];
    char description[100];
    struct Expense *next;
} Expense;

typedef struct StackNode {
    Expense data;
    struct StackNode *next;
} StackNode;

typedef struct QueueNode {
    Expense data;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front, *rear;
} Queue;

Expense *head = NULL;
int nextId = 1;
StackNode *undoStack = NULL;
Queue history = {NULL, NULL};


void saveToFile() {
    FILE *fp = fopen(FILE_NAME, "w");
    if (!fp) return;
    fprintf(fp, "id,date,amount,category,description\n");
    Expense *curr = head;
    while (curr) {
        fprintf(fp, "%d,%s,%.2f,%s,%s\n",
                curr->id, curr->date, curr->amount, curr->category, curr->description);
        curr = curr->next;
    }
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) return;
    char line[256];
    fgets(line, sizeof(line), fp); // skip header
    while (fgets(line, sizeof(line), fp)) {
        Expense *newExp = (Expense *)malloc(sizeof(Expense));
        sscanf(line, "%d,%19[^,],%lf,%49[^,],%99[^\n]",
               &newExp->id, newExp->date, &newExp->amount, newExp->category, newExp->description);
        newExp->next = head;
        head = newExp;
        if (newExp->id >= nextId) nextId = newExp->id + 1;
    }
    fclose(fp);
}


void pushUndo(Expense e) {
    StackNode *node = (StackNode *)malloc(sizeof(StackNode));
    node->data = e;
    node->next = undoStack;
    undoStack = node;
}

int popUndo(Expense *e) {
    if (!undoStack) return 0;
    StackNode *temp = undoStack;
    *e = temp->data;
    undoStack = temp->next;
    free(temp);
    return 1;
}

void enqueue(Expense e) {
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    node->data = e;
    node->next = NULL;
    if (!history.rear)
        history.front = history.rear = node;
    else {
        history.rear->next = node;
        history.rear = node;
    }

    int count = 0;
    QueueNode *q = history.front;
    while (q) { count++; q = q->next; }
    if (count > 5) {
        QueueNode *temp = history.front;
        history.front = history.front->next;
        free(temp);
    }
}

void displayQueue() {
    QueueNode *q = history.front;
    printf("\nRecent Transactions:\n");
    while (q) {
        printf("%d | %s | %.2f | %s | %s\n",
               q->data.id, q->data.date, q->data.amount,
               q->data.category, q->data.description);
        q = q->next;
    }
}


void addExpense() {
    Expense *newExp = (Expense *)malloc(sizeof(Expense));
    newExp->id = nextId++;

    printf("Enter date (YYYY-MM-DD): ");
    scanf("%s", newExp->date);
    printf("Enter amount: ");
    scanf("%lf", &newExp->amount);
    printf("Enter category: ");
    scanf("%s", newExp->category);
    printf("Enter description: ");
    getchar();
    fgets(newExp->description, 100, stdin);
    newExp->description[strcspn(newExp->description, "\n")] = 0;

    newExp->next = head;
    head = newExp;

    enqueue(*newExp);
    saveToFile();
    printf("Expense added successfully!\n");
}

void viewExpenses() {
    Expense *curr = head;
    if (!curr) {
        printf("No expenses recorded.\n");
        return;
    }
    printf("\nID | Date | Amount | Category | Description\n");
   
    while (curr) {
        printf("%d | %s | %.2f | %s | %s\n",
               curr->id, curr->date, curr->amount, curr->category, curr->description);
        curr = curr->next;
    }
}

void deleteExpense() {
    int id;
    printf("Enter ID to delete: ");
    scanf("%d", &id);

    Expense *curr = head, *prev = NULL;
    while (curr && curr->id != id) {
        prev = curr;
        curr = curr->next;
    }
    if (!curr) {
        printf("Expense not found!\n");
        return;
    }

    if (prev) prev->next = curr->next;
    else head = curr->next;

    pushUndo(*curr);
    free(curr);
    saveToFile();
    printf("Expense deleted\n");
}

void undoDelete() {
    Expense e;
    if (popUndo(&e)) {
        Expense *node = (Expense *)malloc(sizeof(Expense));
        *node = e;
        node->next = head;
        head = node;
        saveToFile();
        printf("Expense restored.\n");
    } else {
        printf("Nothing to undo.\n");
    }
}


void calculateTotal() {
    double total = 0;
    Expense *curr = head;
    while (curr) {
        total += curr->amount;
        curr = curr->next;
    }
   
    printf(" Total Expenses: %.2f\n", total);
}  

int main() {
    loadFromFile();
    int choice;
    do {
        printf("\n--- EXPENSE TRACKER ---\n");
        printf("1. Add Expense\n");
        printf("2. View Expenses\n");
        printf("3. Delete Expense\n");
        printf("4. Undo Delete\n");
        printf("5. View Recent \n");
        printf("6. Exit\n");
        printf("7. Total Expenses");
        printf("\nEnter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: addExpense(); break;
            case 2: viewExpenses(); break;
            case 3: deleteExpense(); break;
            case 4: undoDelete(); break;
            case 5: displayQueue(); break;
            case 6: printf("Exiting...\n"); break;
            case 7: calculateTotal(); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 6);
    return 0;
}