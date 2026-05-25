#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 50
#define PHONE_MAX 15
#define FILE_NAME "phonebook.dat"
#define PASS "Admin123"

struct Contact {
    int id;
    char name[MAX];
    char phone[PHONE_MAX];
    char email[60];
    char city[40];
    struct Contact *next;
};

struct Contact *head = NULL;
int total = 0;

void clr() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause() {
    printf("\nPress ENTER to continue...");
    while(getchar() != '\n');
    getchar();
}

void header(char *title) {
    clr();
    printf("========================================\n");
    printf("     PHONEBOOK MANAGEMENT SYSTEM\n");
    printf("========================================\n");
    printf("  %s\n", title);
    printf("========================================\n\n");
}

void getstr(char *msg, char *buf, int size) {
    printf("  %s: ", msg);
    fgets(buf, size, stdin);
    int len = strlen(buf);
    if(len > 0 && buf[len-1] == '\n')
        buf[len-1] = '\0';
}

// checks if needle is inside haystack, case insensitive
int containsStr(char *haystack, char *needle) {
    char h[100], n[100];
    int i;
    for(i = 0; haystack[i]; i++) h[i] = tolower(haystack[i]);
    h[i] = '\0';
    for(i = 0; needle[i]; i++) n[i] = tolower(needle[i]);
    n[i] = '\0';
    return strstr(h, n) != NULL;
}

int validPhone(char *phone) {
    int len = strlen(phone);
    if(len < 7 || len > 13) return 0;
    for(int i = 0; i < len; i++)
        if(!isdigit(phone[i])) return 0;
    return 1;
}

int validEmail(char *email) {
    char *a = strchr(email, '@');
    char *d = strrchr(email, '.');
    return (a != NULL && d != NULL && d > a+1);
}

int phoneUnique(char *phone) {
    struct Contact *c = head;
    while(c != NULL) {
        if(strcmp(c->phone, phone) == 0) return 0;
        c = c->next;
    }
    return 1;
}

void saveFile() {
    FILE *fp = fopen(FILE_NAME, "wb");
    if(!fp) { printf("  Error saving!\n"); return; }
    struct Contact *c = head;
    while(c != NULL) {
        // write everything except the pointer
        fwrite(&c->id,    sizeof(int),  1, fp);
        fwrite(c->name,   sizeof(char), MAX, fp);
        fwrite(c->phone,  sizeof(char), PHONE_MAX, fp);
        fwrite(c->email,  sizeof(char), 60, fp);
        fwrite(c->city,   sizeof(char), 40, fp);
        c = c->next;
    }
    fclose(fp);
}

void loadFile() {
    FILE *fp = fopen(FILE_NAME, "rb");
    if(!fp) return;

    struct Contact *tail = NULL;
    while(1) {
        struct Contact *node = malloc(sizeof(struct Contact));
        node->next = NULL;
        int ok = 1;
        ok &= fread(&node->id,   sizeof(int),  1, fp) == 1;
        ok &= fread(node->name,  sizeof(char), MAX, fp) == MAX;
        ok &= fread(node->phone, sizeof(char), PHONE_MAX, fp) == PHONE_MAX;
        ok &= fread(node->email, sizeof(char), 60, fp) == 60;
        ok &= fread(node->city,  sizeof(char), 40, fp) == 40;

        if(!ok) { free(node); break; }

        if(tail == NULL) { head = node; tail = node; }
        else { tail->next = node; tail = node; }
        total++;
    }
    fclose(fp);
    if(total > 0) printf("  Loaded %d contact(s).\n", total);
}

void addContact() {
    header("Add New Contact");
    struct Contact *node = malloc(sizeof(struct Contact));
    node->id = total + 1;
    node->next = NULL;

    getstr("Name", node->name, MAX);
    if(strlen(node->name) == 0) {
        printf("  Name cant be empty!\n");
        free(node); pause(); return;
    }

    while(1) {
        getstr("Phone (digits only, 7-13)", node->phone, PHONE_MAX);
        if(!validPhone(node->phone)) { printf("  Invalid phone number!\n"); continue; }
        if(!phoneUnique(node->phone)) { printf("  Phone already exists!\n"); continue; }
        break;
    }

    while(1) {
        getstr("Email", node->email, 60);
        if(!validEmail(node->email)) { printf("  Invalid email!\n"); continue; }
        break;
    }

    getstr("City", node->city, 40);
    if(strlen(node->city) == 0) {
        printf("  City cant be empty!\n");
        free(node); pause(); return;
    }

    if(head == NULL) {
        head = node;
    } else {
        struct Contact *c = head;
        while(c->next != NULL) c = c->next;
        c->next = node;
    }
    total++;
    saveFile();
    printf("\n  Contact added successfully! ID: %d\n", node->id);
    pause();
}

void displayAll() {
    header("All Contacts");
    if(head == NULL) { printf("  No contacts found.\n"); pause(); return; }

    printf("  %-5s %-20s %-13s %-25s %-15s\n", "ID", "Name", "Phone", "Email", "City");
    printf("  -----------------------------------------------------------------------\n");
    struct Contact *c = head;
    while(c != NULL) {
        printf("  %-5d %-20s %-13s %-25s %-15s\n",
               c->id, c->name, c->phone, c->email, c->city);
        c = c->next;
    }
    printf("  -----------------------------------------------------------------------\n");
    printf("  Total contacts: %d\n", total);
    pause();
}

void editContact() {
    header("Edit Contact");
    if(head == NULL) { printf("  No contacts.\n"); pause(); return; }

    int id;
    printf("  Enter contact ID to edit: ");
    scanf("%d", &id);
    while(getchar() != '\n');

    struct Contact *c = head;
    while(c != NULL) {
        if(c->id == id) {
            printf("\n  Name: %s | Phone: %s | City: %s\n", c->name, c->phone, c->city);
            printf("  (Press ENTER to keep current value)\n\n");

            char buf[60];

            getstr("New Name", buf, MAX);
            if(strlen(buf) > 0) strcpy(c->name, buf);

            while(1) {
                getstr("New Phone", buf, PHONE_MAX);
                if(strlen(buf) == 0) break;
                if(!validPhone(buf)) { printf("  Invalid phone!\n"); continue; }
                if(strcmp(buf, c->phone) != 0 && !phoneUnique(buf)) {
                    printf("  Number already used!\n"); continue;
                }
                strcpy(c->phone, buf);
                break;
            }

            getstr("New Email", buf, 60);
            if(strlen(buf) > 0 && validEmail(buf)) strcpy(c->email, buf);

            getstr("New City", buf, 40);
            if(strlen(buf) > 0) strcpy(c->city, buf);

            saveFile();
            printf("\n  Updated!\n");
            pause(); return;
        }
        c = c->next;
    }
    printf("\n  Contact with ID %d not found.\n", id);
    pause();
}

void deleteContact() {
    header("Delete Contact");
    if(head == NULL) { printf("  No contacts.\n"); pause(); return; }

    int id;
    printf("  Enter contact ID to delete: ");
    scanf("%d", &id);
    while(getchar() != '\n');

    struct Contact *c = head, *prev = NULL;
    while(c != NULL) {
        if(c->id == id) {
            if(prev == NULL) head = c->next;
            else prev->next = c->next;
            printf("\n  Deleted: %s\n", c->name);
            free(c);
            total--;
            saveFile();
            pause(); return;
        }
        prev = c;
        c = c->next;
    }
    printf("\n  ID not found.\n");
    pause();
}

void deleteAll() {
    header("Delete All");
    if(head == NULL) { printf("  Already empty.\n"); pause(); return; }

    char confirm[5];
    printf("  Type YES to confirm deleting all %d contacts: ", total);
    fgets(confirm, 5, stdin);
    confirm[strcspn(confirm, "\n")] = '\0';

    if(strcmp(confirm, "YES") != 0) {
        printf("\n  Cancelled.\n"); pause(); return;
    }
    struct Contact *c = head;
    while(c != NULL) {
        struct Contact *tmp = c;
        c = c->next;
        free(tmp);
    }
    head = NULL;
    total = 0;
    saveFile();
    printf("\n  All contacts deleted.\n");
    pause();
}

int nameCmp(char *a, char *b) {
    while(*a && *b) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if(d != 0) return d;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

struct Contact *mergeLists(struct Contact *a, struct Contact *b) {
    if(!a) return b;
    if(!b) return a;
    if(nameCmp(a->name, b->name) <= 0) {
        a->next = mergeLists(a->next, b);
        return a;
    }
    b->next = mergeLists(a, b->next);
    return b;
}

struct Contact *getMiddle(struct Contact *node) {
    struct Contact *slow = node, *fast = node->next;
    while(fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
}

struct Contact *mergeSort(struct Contact *node) {
    if(!node || !node->next) return node;
    struct Contact *mid = getMiddle(node);
    struct Contact *right = mid->next;
    mid->next = NULL;
    return mergeLists(mergeSort(node), mergeSort(right));
}

void sortContacts() {
    if(!head) { printf("\n  No contacts to sort.\n"); return; }
    head = mergeSort(head);
    saveFile();
    printf("\n  Contacts sorted A to Z!\n");
}

void printOne(struct Contact *c) {
    printf("\n  ID    : %d\n", c->id);
    printf("  Name  : %s\n", c->name);
    printf("  Phone : %s\n", c->phone);
    printf("  Email : %s\n", c->email);
    printf("  City  : %s\n", c->city);
    printf("  -------------------------\n");
}

void searchByName() {
    header("Search by Name");
    if(!head) { printf("  No contacts.\n"); pause(); return; }
    char q[MAX];
    getstr("Enter name", q, MAX);
    int found = 0;
    struct Contact *c = head;
    while(c != NULL) {
        if(containsStr(c->name, q)) { printOne(c); found++; }
        c = c->next;
    }
    if(!found) printf("\n  No results for '%s'\n", q);
    else printf("\n  %d result(s) found.\n", found);
    pause();
}

void searchByPhone() {
    header("Search by Phone");
    if(!head) { printf("  No contacts.\n"); pause(); return; }
    char q[PHONE_MAX];
    getstr("Enter phone number", q, PHONE_MAX);
    struct Contact *c = head;
    while(c != NULL) {
        if(strcmp(c->phone, q) == 0) { printOne(c); pause(); return; }
        c = c->next;
    }
    printf("\n  Not found.\n");
    pause();
}

void searchByCity() {
    header("Search by City");
    if(!head) { printf("  No contacts.\n"); pause(); return; }
    char q[40];
    getstr("Enter city", q, 40);
    int found = 0;
    struct Contact *c = head;
    while(c != NULL) {
        if(containsStr(c->city, q)) { printOne(c); found++; }
        c = c->next;
    }
    if(!found) printf("\n  No results for '%s'\n", q);
    else printf("\n  %d result(s) found.\n", found);
    pause();
}

int adminLogin() {
    header("Admin Login");
    char pass[30];
    int tries = 3;
    while(tries > 0) {
        printf("  Password (%d tries left): ", tries);
        fgets(pass, 30, stdin);
        pass[strcspn(pass, "\n")] = '\0';
        if(strcmp(pass, PASS) == 0) {
            printf("\n  Login successful!\n");
            pause(); return 1;
        }
        tries--;
        printf("  Wrong password!\n\n");
    }
    printf("  Too many wrong attempts.\n");
    pause();
    return 0;
}

void adminMenu() {
    int ch;
    do {
        header("Admin Panel");
        printf("  1. Add Contact\n");
        printf("  2. View All\n");
        printf("  3. Edit Contact\n");
        printf("  4. Delete Contact\n");
        printf("  5. Delete All\n");
        printf("  6. Sort A to Z\n");
        printf("  7. Search by Name\n");
        printf("  8. Search by Phone\n");
        printf("  9. Search by City\n");
        printf("  0. Logout\n\n");
        printf("  Enter choice: ");
        scanf("%d", &ch);
        while(getchar() != '\n');

        switch(ch) {
            case 1: addContact(); break;
            case 2: displayAll(); break;
            case 3: editContact(); break;
            case 4: deleteContact(); break;
            case 5: deleteAll(); break;
            case 6: sortContacts(); pause(); break;
            case 7: searchByName(); break;
            case 8: searchByPhone(); break;
            case 9: searchByCity(); break;
            case 0: printf("\n  Logged out.\n"); pause(); break;
            default: printf("\n  Invalid choice.\n"); pause();
        }
    } while(ch != 0);
}

void userMenu() {
    int ch;
    do {
        header("User Panel");
        printf("  1. View All Contacts\n");
        printf("  2. Search by Name\n");
        printf("  3. Search by Phone\n");
        printf("  4. Search by City\n");
        printf("  0. Back\n\n");
        printf("  Enter choice: ");
        scanf("%d", &ch);
        while(getchar() != '\n');

        switch(ch) {
            case 1: displayAll(); break;
            case 2: searchByName(); break;
            case 3: searchByPhone(); break;
            case 4: searchByCity(); break;
            case 0: break;
            default: printf("\n  Invalid.\n"); pause();
        }
    } while(ch != 0);
}

int main() {
    printf("\n  Loading phonebook...\n");
    loadFile();

    int ch;
    do {
        header("Welcome");
        printf("  1. Admin Login\n");
        printf("  2. User Access\n");
        printf("  0. Exit\n\n");
        printf("  Enter choice: ");
        scanf("%d", &ch);
        while(getchar() != '\n');

        switch(ch) {
            case 1: if(adminLogin()) adminMenu(); break;
            case 2: userMenu(); break;
            case 0: printf("\n  Goodbye!\n"); break;
            default: printf("\n  Invalid.\n"); pause();
        }
    } while(ch != 0);

    return 0;
}
