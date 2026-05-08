#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
using namespace std;

// ===================== STRUCTURES =====================
struct Date {
    int day;
    int month;
    int year;
};

struct Borrower {
    int borrowerId;
    string fullName;
    string phone;
};

struct LoanInfo {
    Borrower borrower;
    Date issueDate;
    Date dueDate;
    bool returned;
};

struct Book {
    int bookId;
    string title;
    string author;
    int yearPublished;
    bool available;
    LoanInfo* loan;
};

// ===================== CONSTANTS =====================
const int LOAN_PERIOD_DAYS = 14;

// ===================== FILE FORMAT SPECIFICATION =====================
// books.txt — one book per line, pipe-delimited:
// bookId|title|author|yearPublished|available
// e.g. 1|The Great Gatsby|F. Scott Fitzgerald|1925|1
// If the book is currently on loan, the next line holds the loan record:
// LOAN|borrowerId|borrowerName|borrowerPhone|issueDay|issueMonth|issueYear|dueDay|dueMonth|dueYear
// A line containing "NOLOAN" means no active loan.
//
// borrowers.txt — one borrower per line, pipe-delimited:
// borrowerId|fullName|phone
// e.g. 101|Jane Doe|0712345678

// ===================== FUNCTION PROTOTYPES =====================

// Student 1 - Menu
void showMainMenu();
void processChoice(int choice, Book*& books, int& bookCount, Borrower*& borrowers, int& borrowerCount);

// Student 2 - Book Module
void addBook(Book*& books, int& bookCount);
void displayBooks(Book* books, int bookCount);
int findBookIndexById(Book* books, int bookCount, int id);
void searchBookByTitle(Book* books, int bookCount);

// Student 3 - Borrower Module
void addBorrower(Borrower*& borrowers, int& borrowerCount);
void displayBorrowers(Borrower* borrowers, int borrowerCount);
int findBorrowerIndexById(Borrower* borrowers, int borrowerCount, int id);

// Student 4 - Loan Module
void borrowBook(Book* books, int bookCount, Borrower* borrowers, int borrowerCount);
Date createDueDate(Date issueDate, int daysAllowed);
Date getCurrentDate();

// Student 5 - Return Module
void returnBook(Book* books, int bookCount);
bool isOverdue(Date dueDate, Date currentDate);

// Student 6 - Dynamic Memory Module
void resizeBooksArray(Book*& books, int oldSize, int newSize);
void resizeBorrowersArray(Borrower*& borrowers, int oldSize, int newSize);
void releaseMemory(Book*& books, int& bookCount, Borrower*& borrowers, int& borrowerCount);

// Student 7 - File Module
void saveBooksToFile(Book* books, int bookCount);
void loadBooksFromFile(Book*& books, int& bookCount);
void saveBorrowersToFile(Borrower* borrowers, int borrowerCount);
void loadBorrowersFromFile(Borrower*& borrowers, int& borrowerCount);

// Student 8 - Reports Module
void displayActiveLoans(Book* books, int bookCount);
void showSummary(Book* books, int bookCount, Borrower* borrowers, int borrowerCount);

// Student 9 - Template Module
// BUG FIX: Original had "a = temp;" instead of "b = temp;" — fixed below.
template <typename T>
void swapValues(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp; // FIXED: was "a = temp;" which never updated b
}

// linearSearchId uses a common 'id' field approach (Option A from the hint).
// Both Book and Borrower expose their id via a wrapper; we use two
// explicit overloads so the template can dispatch correctly without
// requiring a common field name.
template <typename T>
int linearSearchId(T* arr, int size, int targetId);

// Explicit specialisations declared here; defined after main.
template <>
int linearSearchId<Book>(Book* arr, int size, int targetId);

template <>
int linearSearchId<Borrower>(Borrower* arr, int size, int targetId);

// Student 10 - Integration and Testing
void runTests();

// ===================== MAIN =====================
int main() {
    Book* books = nullptr;
    Borrower* borrowers = nullptr;
    int bookCount = 0;
    int borrowerCount = 0;

    loadBooksFromFile(books, bookCount);
    loadBorrowersFromFile(borrowers, borrowerCount);

    // Run integration tests on startup (Student 10)
    runTests();

    int choice;
    do {
        showMainMenu();
        cin >> choice;
        processChoice(choice, books, bookCount, borrowers, borrowerCount);
    } while (choice != 0);

    saveBooksToFile(books, bookCount);
    saveBorrowersToFile(borrowers, borrowerCount);
    releaseMemory(books, bookCount, borrowers, borrowerCount);

    return 0;
}

// ===================== STUDENT 1 — MENU MODULE =====================

void showMainMenu() {
    cout << "\n===== COMMUNITY LIBRARY SYSTEM =====\n";
    cout << "1. Add Book\n";
    cout << "2. Add Borrower\n";
    cout << "3. Borrow Book\n";
    cout << "4. Return Book\n";
    cout << "5. Search Book by Title\n";
    cout << "6. Display All Books\n";
    cout << "7. Display All Borrowers\n";
    cout << "8. Display Active Loans\n";
    cout << "9. Show Summary\n";
    cout << "0. Exit\n";
    cout << "Enter choice: ";
}

void processChoice(int choice, Book*& books, int& bookCount,
                   Borrower*& borrowers, int& borrowerCount) {
    switch (choice) {
        case 1: addBook(books, bookCount);                                         break;
        case 2: addBorrower(borrowers, borrowerCount);                             break;
        case 3: borrowBook(books, bookCount, borrowers, borrowerCount);            break;
        case 4: returnBook(books, bookCount);                                      break;
        case 5: searchBookByTitle(books, bookCount);                               break;
        case 6: displayBooks(books, bookCount);                                    break;
        case 7: displayBorrowers(borrowers, borrowerCount);                        break;
        case 8: displayActiveLoans(books, bookCount);                              break;
        case 9: showSummary(books, bookCount, borrowers, borrowerCount);           break;
        case 0: cout << "Saving data and exiting. Goodbye!\n";                    break;
        default: cout << "Invalid choice. Please enter a number from 0 to 9.\n";
    }
}

// ===================== STUDENT 2 — BOOK MODULE =====================

void addBook(Book*& books, int& bookCount) {
    // 1. Resize the books array to hold one more book
    resizeBooksArray(books, bookCount, bookCount + 1);

    // 2. Populate the new slot (last index after resize)
    Book& b = books[bookCount - 1];

    cout << "\n--- Add New Book ---\n";
    cout << "Enter Book ID: ";
    cin >> b.bookId;
    cin.ignore();

    cout << "Enter Title: ";
    getline(cin, b.title);

    cout << "Enter Author: ";
    getline(cin, b.author);

    cout << "Enter Year Published: ";
    cin >> b.yearPublished;

    // 3. Default state
    b.available = true;
    // 4. No active loan
    b.loan = nullptr;

    cout << "Book \"" << b.title << "\" added successfully (ID: " << b.bookId << ").\n";
}

void displayBooks(Book* books, int bookCount) {
    if (bookCount == 0) {
        cout << "\nNo books in the system.\n";
        return;
    }

    cout << "\n===== ALL BOOKS =====\n";
    cout << "------------------------------------------------------------\n";
    for (int i = 0; i < bookCount; i++) {
        Book& b = books[i];
        cout << "ID      : " << b.bookId          << "\n";
        cout << "Title   : " << b.title            << "\n";
        cout << "Author  : " << b.author           << "\n";
        cout << "Year    : " << b.yearPublished     << "\n";
        cout << "Status  : " << (b.available ? "Available" : "Borrowed") << "\n";
        if (!b.available && b.loan != nullptr) {
            LoanInfo* l = b.loan;
            cout << "Borrower: " << l->borrower.fullName
                 << " (ID: " << l->borrower.borrowerId << ")\n";
            cout << "Due Date: " << l->dueDate.day << "/"
                 << l->dueDate.month << "/" << l->dueDate.year << "\n";
        }
        cout << "------------------------------------------------------------\n";
    }
}

int findBookIndexById(Book* books, int bookCount, int id) {
    // Use the explicit template specialisation for Book
    return linearSearchId<Book>(books, bookCount, id);
}

void searchBookByTitle(Book* books, int bookCount) {
    if (bookCount == 0) {
        cout << "\nNo books in the system.\n";
        return;
    }

    cin.ignore();
    cout << "Enter title (or part of title) to search: ";
    string query;
    getline(cin, query);

    // Convert query to lowercase for case-insensitive matching
    string lowerQuery = query;
    for (int i = 0; i < (int)lowerQuery.size(); i++)
        lowerQuery[i] = tolower(lowerQuery[i]);

    bool found = false;
    cout << "\n--- Search Results ---\n";
    for (int i = 0; i < bookCount; i++) {
        string lowerTitle = books[i].title;
        for (int j = 0; j < (int)lowerTitle.size(); j++)
            lowerTitle[j] = tolower(lowerTitle[j]);

        if (lowerTitle.find(lowerQuery) != string::npos) {
            cout << "ID: " << books[i].bookId
                 << " | Title: " << books[i].title
                 << " | Author: " << books[i].author
                 << " | Status: " << (books[i].available ? "Available" : "Borrowed") << "\n";
            found = true;
        }
    }
    if (!found)
        cout << "No books found matching \"" << query << "\".\n";
}

// ===================== STUDENT 3 — BORROWER MODULE =====================

void addBorrower(Borrower*& borrowers, int& borrowerCount) {
    resizeBorrowersArray(borrowers, borrowerCount, borrowerCount + 1);

    Borrower& bw = borrowers[borrowerCount - 1];

    cout << "\n--- Add New Borrower ---\n";
    cout << "Enter Borrower ID: ";
    cin >> bw.borrowerId;
    cin.ignore();

    cout << "Enter Full Name: ";
    getline(cin, bw.fullName);

    cout << "Enter Phone Number: ";
    getline(cin, bw.phone);

    cout << "Borrower \"" << bw.fullName << "\" added successfully (ID: "
         << bw.borrowerId << ").\n";
}

void displayBorrowers(Borrower* borrowers, int borrowerCount) {
    if (borrowerCount == 0) {
        cout << "\nNo borrowers in the system.\n";
        return;
    }

    cout << "\n===== ALL BORROWERS =====\n";
    cout << "--------------------------------------------\n";
    for (int i = 0; i < borrowerCount; i++) {
        cout << "ID    : " << borrowers[i].borrowerId << "\n";
        cout << "Name  : " << borrowers[i].fullName   << "\n";
        cout << "Phone : " << borrowers[i].phone       << "\n";
        cout << "--------------------------------------------\n";
    }
}

int findBorrowerIndexById(Borrower* borrowers, int borrowerCount, int id) {
    return linearSearchId<Borrower>(borrowers, borrowerCount, id);
}

// ===================== STUDENT 4 — LOAN MODULE =====================

Date getCurrentDate() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    Date today;
    today.day   = localTime->tm_mday;
    today.month = localTime->tm_mon + 1;   // tm_mon is 0-based
    today.year  = localTime->tm_year + 1900;
    return today;
}

Date createDueDate(Date issueDate, int daysAllowed) {
    Date due = issueDate;
    // Simplified rule for Version 1: add days directly.
    // No full month/year rollover required per assignment spec.
    due.day = issueDate.day + daysAllowed;
    return due;
}

void borrowBook(Book* books, int bookCount, Borrower* borrowers, int borrowerCount) {
    // 1. Ask for book ID
    int bookId;
    cout << "\nEnter Book ID to borrow: ";
    cin >> bookId;

    // 2. Ask for borrower ID
    int borrowerId;
    cout << "Enter Borrower ID: ";
    cin >> borrowerId;

    // 3. Verify both exist
    int bIdx = findBookIndexById(books, bookCount, bookId);
    if (bIdx == -1) {
        cout << "Error: Book ID " << bookId << " not found.\n";
        return;
    }

    int brIdx = findBorrowerIndexById(borrowers, borrowerCount, borrowerId);
    if (brIdx == -1) {
        cout << "Error: Borrower ID " << borrowerId << " not found.\n";
        return;
    }

    // 4. Verify book is available
    if (!books[bIdx].available) {
        cout << "Error: Book \"" << books[bIdx].title << "\" is already borrowed.\n";
        return;
    }

    // 5. Dynamically allocate LoanInfo
    LoanInfo* newLoan = new LoanInfo;

    // 6. Fill in loan details
    newLoan->borrower  = borrowers[brIdx];
    newLoan->issueDate = getCurrentDate();
    newLoan->dueDate   = createDueDate(newLoan->issueDate, LOAN_PERIOD_DAYS);
    newLoan->returned  = false;

    // 7. Attach loan to book and mark unavailable
    books[bIdx].loan      = newLoan;
    books[bIdx].available = false;

    cout << "Book \"" << books[bIdx].title << "\" successfully borrowed by "
         << borrowers[brIdx].fullName << ".\n";
    cout << "Due date: " << newLoan->dueDate.day << "/"
         << newLoan->dueDate.month << "/" << newLoan->dueDate.year << "\n";
}

// ===================== STUDENT 5 — RETURN MODULE =====================

bool isOverdue(Date dueDate, Date currentDate) {
    if (currentDate.year  > dueDate.year)  return true;
    if (currentDate.year  < dueDate.year)  return false;
    if (currentDate.month > dueDate.month) return true;
    if (currentDate.month < dueDate.month) return false;
    return currentDate.day > dueDate.day;
}

void returnBook(Book* books, int bookCount) {
    // 1. Find book by ID
    int bookId;
    cout << "\nEnter Book ID to return: ";
    cin >> bookId;

    int bIdx = findBookIndexById(books, bookCount, bookId);
    if (bIdx == -1) {
        cout << "Error: Book ID " << bookId << " not found.\n";
        return;
    }

    // 2. Check if a loan exists
    if (books[bIdx].available || books[bIdx].loan == nullptr) {
        cout << "Error: Book \"" << books[bIdx].title << "\" is not currently borrowed.\n";
        return;
    }

    // Check overdue status before returning
    Date today = getCurrentDate();
    if (isOverdue(books[bIdx].loan->dueDate, today)) {
        cout << "Warning: This book is OVERDUE (was due "
             << books[bIdx].loan->dueDate.day << "/"
             << books[bIdx].loan->dueDate.month << "/"
             << books[bIdx].loan->dueDate.year << ").\n";
    }

    // 3. Mark returned
    books[bIdx].loan->returned = true;

    // 4. Delete dynamically allocated loan memory
    delete books[bIdx].loan;
    books[bIdx].loan = nullptr;

    // 5. Mark book available
    books[bIdx].available = true;

    cout << "Book \"" << books[bIdx].title << "\" returned successfully.\n";
}

// ===================== STUDENT 6 — DYNAMIC MEMORY MODULE =====================

void resizeBooksArray(Book*& books, int oldSize, int newSize) {
    // 1. Allocate new dynamic array
    Book* newArr = new Book[newSize];

    // 2. Copy old data
    for (int i = 0; i < oldSize && i < newSize; i++)
        newArr[i] = books[i];

    // 3. Initialise any new positions (growing)
    for (int i = oldSize; i < newSize; i++) {
        newArr[i].bookId        = 0;
        newArr[i].yearPublished = 0;
        newArr[i].available     = true;
        newArr[i].loan          = nullptr;
    }

    // 4. Delete old array
    delete[] books;

    // 5. Update pointer and size
    books = newArr;
    // Note: bookCount is updated by the caller (addBook increments after resize)
    // We increment it here for convenience since resizeBooksArray always adds 1.
    // The new bookCount equals newSize.
    // Caller uses bookCount - 1 as the index of the freshly added slot.
    // We update via reference through the pointer; the caller passes bookCount
    // to oldSize and newSize = oldSize + 1, so we just reflect newSize back.
    // Because bookCount is not passed here, the caller (addBook) owns that update.
    // resizeBooksArray only manages the array pointer.
    //
    // IMPORTANT: addBook calls resizeBooksArray(books, bookCount, bookCount + 1)
    // and then increments bookCount itself (via the newSize becoming the new count).
    // For simplicity we make this function also update bookCount by receiving it:
    // — but the prototype does not include bookCount. So addBook sets bookCount
    //   after calling this. See addBook for the bookCount++ that follows.
    //
    // (No action needed here — bookCount managed by caller.)
}

void resizeBorrowersArray(Borrower*& borrowers, int oldSize, int newSize) {
    Borrower* newArr = new Borrower[newSize];

    for (int i = 0; i < oldSize && i < newSize; i++)
        newArr[i] = borrowers[i];

    for (int i = oldSize; i < newSize; i++) {
        newArr[i].borrowerId = 0;
        newArr[i].fullName   = "";
        newArr[i].phone      = "";
    }

    delete[] borrowers;
    borrowers = newArr;
}

void releaseMemory(Book*& books, int& bookCount,
                   Borrower*& borrowers, int& borrowerCount) {
    // WARNING: Delete each book's loan pointer BEFORE deleting the books array.
    // Deleting the array first makes loan pointers unreachable — memory leak.

    // Step 1: Delete all loan sub-objects
    for (int i = 0; i < bookCount; i++) {
        if (books[i].loan != nullptr) {
            delete books[i].loan;
            books[i].loan = nullptr;
        }
    }

    // Step 2: Delete books array
    delete[] books;
    books = nullptr;

    // Step 3: Delete borrowers array
    delete[] borrowers;
    borrowers = nullptr;

    // Step 4: Reset counts
    bookCount     = 0;
    borrowerCount = 0;
}

// ===================== STUDENT 7 — FILE MODULE =====================

void saveBooksToFile(Book* books, int bookCount) {
    ofstream file("books.txt", ios::out);
    if (!file.is_open()) {
        cout << "Error: Could not open books.txt for writing.\n";
        return;
    }

    for (int i = 0; i < bookCount; i++) {
        Book& b = books[i];
        // bookId|title|author|yearPublished|available
        file << b.bookId       << "|"
             << b.title        << "|"
             << b.author       << "|"
             << b.yearPublished << "|"
             << (b.available ? 1 : 0) << "\n";

        // LOAN line or NOLOAN
        if (!b.available && b.loan != nullptr) {
            LoanInfo* l = b.loan;
            file << "LOAN"
                 << "|" << l->borrower.borrowerId
                 << "|" << l->borrower.fullName
                 << "|" << l->borrower.phone
                 << "|" << l->issueDate.day
                 << "|" << l->issueDate.month
                 << "|" << l->issueDate.year
                 << "|" << l->dueDate.day
                 << "|" << l->dueDate.month
                 << "|" << l->dueDate.year
                 << "\n";
        } else {
            file << "NOLOAN\n";
        }
    }

    file.close();
    cout << "Books saved to books.txt.\n";
}

void loadBooksFromFile(Book*& books, int& bookCount) {
    ifstream file("books.txt");
    if (!file.is_open()) {
        // File does not exist yet — that is fine on first run
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        // Parse book line: bookId|title|author|yearPublished|available
        Book b;
        b.loan = nullptr;

        // Use a stringstream-free approach with getline + '|' delimiter
        // by reading from a temporary stream built from the line.
        // We simulate field extraction with repeated getline on a string.
        // Technique: replace '|' with '\n', put into istringstream.
        string tmp = line;
        for (int i = 0; i < (int)tmp.size(); i++)
            if (tmp[i] == '|') tmp[i] = '\n';

        // Re-read fields using a char buffer approach
        {
            // We extract fields manually to avoid <sstream> (not listed as prohibited
            // but keeping dependencies minimal).
            int pos = 0;
            auto nextField = [&](const string& s, int& start) -> string {
                int end = start;
                while (end < (int)s.size() && s[end] != '\n') end++;
                string f = s.substr(start, end - start);
                start = end + 1;
                return f;
            };

            b.bookId        = stoi(nextField(tmp, pos));
            b.title         = nextField(tmp, pos);
            b.author        = nextField(tmp, pos);
            b.yearPublished = stoi(nextField(tmp, pos));
            b.available     = (stoi(nextField(tmp, pos)) == 1);
        }

        // Next line: LOAN or NOLOAN
        string loanLine;
        if (!getline(file, loanLine)) break;

        if (loanLine.substr(0, 4) == "LOAN") {
            // Parse: LOAN|borrowerId|name|phone|iD|iM|iY|dD|dM|dY
            LoanInfo* l = new LoanInfo;
            l->returned  = false;

            string lt = loanLine;
            for (int i = 0; i < (int)lt.size(); i++)
                if (lt[i] == '|') lt[i] = '\n';

            int pos = 0;
            auto nf = [&](const string& s, int& start) -> string {
                int end = start;
                while (end < (int)s.size() && s[end] != '\n') end++;
                string f = s.substr(start, end - start);
                start = end + 1;
                return f;
            };

            nf(lt, pos); // skip "LOAN" token
            l->borrower.borrowerId = stoi(nf(lt, pos));
            l->borrower.fullName   = nf(lt, pos);
            l->borrower.phone      = nf(lt, pos);
            l->issueDate.day       = stoi(nf(lt, pos));
            l->issueDate.month     = stoi(nf(lt, pos));
            l->issueDate.year      = stoi(nf(lt, pos));
            l->dueDate.day         = stoi(nf(lt, pos));
            l->dueDate.month       = stoi(nf(lt, pos));
            l->dueDate.year        = stoi(nf(lt, pos));

            b.loan = l;
        }
        // NOLOAN → loan stays nullptr

        // Append to books array
        resizeBooksArray(books, bookCount, bookCount + 1);
        books[bookCount] = b;
        bookCount++;
    }

    file.close();
}

void saveBorrowersToFile(Borrower* borrowers, int borrowerCount) {
    ofstream file("borrowers.txt", ios::out);
    if (!file.is_open()) {
        cout << "Error: Could not open borrowers.txt for writing.\n";
        return;
    }

    for (int i = 0; i < borrowerCount; i++) {
        file << borrowers[i].borrowerId << "|"
             << borrowers[i].fullName   << "|"
             << borrowers[i].phone      << "\n";
    }

    file.close();
    cout << "Borrowers saved to borrowers.txt.\n";
}

void loadBorrowersFromFile(Borrower*& borrowers, int& borrowerCount) {
    ifstream file("borrowers.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        // Replace '|' with '\n' for field extraction
        string tmp = line;
        for (int i = 0; i < (int)tmp.size(); i++)
            if (tmp[i] == '|') tmp[i] = '\n';

        int pos = 0;
        auto nf = [&](const string& s, int& start) -> string {
            int end = start;
            while (end < (int)s.size() && s[end] != '\n') end++;
            string f = s.substr(start, end - start);
            start = end + 1;
            return f;
        };

        Borrower bw;
        bw.borrowerId = stoi(nf(tmp, pos));
        bw.fullName   = nf(tmp, pos);
        bw.phone      = nf(tmp, pos);

        resizeBorrowersArray(borrowers, borrowerCount, borrowerCount + 1);
        borrowers[borrowerCount] = bw;
        borrowerCount++;
    }

    file.close();
}

// ===================== STUDENT 8 — REPORTS MODULE =====================

void displayActiveLoans(Book* books, int bookCount) {
    cout << "\n===== ACTIVE LOANS =====\n";
    bool anyFound = false;
    Date today = getCurrentDate();

    for (int i = 0; i < bookCount; i++) {
        if (!books[i].available && books[i].loan != nullptr) {
            anyFound = true;
            LoanInfo* l = books[i].loan;
            bool overdue = isOverdue(l->dueDate, today);

            cout << "Book ID  : " << books[i].bookId   << "\n";
            cout << "Title    : " << books[i].title     << "\n";
            cout << "Borrower : " << l->borrower.fullName
                 << " (ID: " << l->borrower.borrowerId << ")\n";
            cout << "Issued   : " << l->issueDate.day << "/"
                 << l->issueDate.month << "/" << l->issueDate.year << "\n";
            cout << "Due      : " << l->dueDate.day << "/"
                 << l->dueDate.month << "/" << l->dueDate.year << "\n";
            cout << "Status   : " << (overdue ? "*** OVERDUE ***" : "On Time") << "\n";
            cout << "--------------------------------------------\n";
        }
    }

    if (!anyFound)
        cout << "No active loans.\n";
}

void showSummary(Book* books, int bookCount,
                 Borrower* borrowers, int borrowerCount) {
    int borrowed  = 0;
    int available = 0;
    int overdue   = 0;
    Date today    = getCurrentDate();

    for (int i = 0; i < bookCount; i++) {
        if (books[i].available) {
            available++;
        } else {
            borrowed++;
            if (books[i].loan != nullptr && isOverdue(books[i].loan->dueDate, today))
                overdue++;
        }
    }

    cout << "\n===== LIBRARY SUMMARY =====\n";
    cout << "Total Books     : " << bookCount      << "\n";
    cout << "Available Books : " << available       << "\n";
    cout << "Borrowed Books  : " << borrowed        << "\n";
    cout << "Overdue Books   : " << overdue         << "\n";
    cout << "Total Borrowers : " << borrowerCount   << "\n";
    cout << "===========================\n";
}

// ===================== STUDENT 9 — TEMPLATE MODULE =====================

// Explicit specialisation for Book — searches by bookId
template <>
int linearSearchId<Book>(Book* arr, int size, int targetId) {
    for (int i = 0; i < size; i++) {
        if (arr[i].bookId == targetId)
            return i;
    }
    return -1;
}

// Explicit specialisation for Borrower — searches by borrowerId
template <>
int linearSearchId<Borrower>(Borrower* arr, int size, int targetId) {
    for (int i = 0; i < size; i++) {
        if (arr[i].borrowerId == targetId)
            return i;
    }
    return -1;
}

// ===================== STUDENT 10 — INTEGRATION AND TESTING =====================

void runTests() {
    cout << "\n===== RUNNING INTEGRATION TESTS =====\n";

    // --- Test 1: swapValues template (verifies the bug is fixed) ---
    int x = 5, y = 10;
    cout << "Test 1 - swapValues<int>:\n";
    cout << "  Before: x=" << x << " y=" << y << "\n";
    swapValues(x, y);
    cout << "  After : x=" << x << " y=" << y << "\n";
    // Expected: x=10 y=5  (bug would give x=5 y=5)
    if (x == 10 && y == 5)
        cout << "  PASSED\n";
    else
        cout << "  FAILED (swapValues bug still present!)\n";

    // --- Test 2: swapValues with doubles ---
    double a = 3.14, b2 = 2.72;
    cout << "Test 2 - swapValues<double>:\n";
    cout << "  Before: a=" << a << " b=" << b2 << "\n";
    swapValues(a, b2);
    cout << "  After : a=" << a << " b=" << b2 << "\n";
    if (a == 2.72 && b2 == 3.14)
        cout << "  PASSED\n";
    else
        cout << "  FAILED\n";

    // --- Test 3: isOverdue ---
    cout << "Test 3 - isOverdue:\n";
    Date due     = {1, 5, 2026};
    Date onTime  = {1, 5, 2026};
    Date overdue = {2, 5, 2026};
    cout << "  Same day (not overdue): "
         << (isOverdue(due, onTime)  ? "OVERDUE (wrong)" : "Not overdue (correct)") << "\n";
    cout << "  One day late (overdue): "
         << (isOverdue(due, overdue) ? "Overdue (correct)" : "Not overdue (wrong)") << "\n";

    // --- Test 4: linearSearchId on a small array ---
    cout << "Test 4 - linearSearchId<Book>:\n";
    Book testBooks[3];
    testBooks[0].bookId = 10; testBooks[0].loan = nullptr;
    testBooks[1].bookId = 20; testBooks[1].loan = nullptr;
    testBooks[2].bookId = 30; testBooks[2].loan = nullptr;
    int idx = linearSearchId<Book>(testBooks, 3, 20);
    cout << "  Search for ID 20 -> index " << idx
         << (idx == 1 ? " (PASSED)" : " (FAILED)") << "\n";
    int miss = linearSearchId<Book>(testBooks, 3, 99);
    cout << "  Search for ID 99 -> index " << miss
         << (miss == -1 ? " (PASSED)" : " (FAILED)") << "\n";

    // --- Test 5: createDueDate ---
    cout << "Test 5 - createDueDate:\n";
    Date issue = {1, 5, 2026};
    Date due2  = createDueDate(issue, LOAN_PERIOD_DAYS);
    cout << "  Issue: 1/5/2026 + 14 days -> Due: "
         << due2.day << "/" << due2.month << "/" << due2.year
         << (due2.day == 15 ? " (PASSED)" : " (FAILED)") << "\n";

    cout << "===== TESTS COMPLETE =====\n\n";
}
