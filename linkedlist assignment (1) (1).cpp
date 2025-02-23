#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip> 
#include <ctime>
#include <windows.h>
#include <psapi.h>
#include <chrono>

using namespace std; 
using namespace std::chrono;


// Define the structure for the linked list
struct news {
    string title;
    string text;
    string subject;
    string date;
    news* address; // Pointer to the next node in the linked list
};


void printMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        cout << "Memory Usage: " << pmc.WorkingSetSize / 1024 << " KB" << endl;
    }
}

// Function to handle quoted CSV fields correctly
string getCSVField(stringstream& ss) {
    string field, temp;
    bool inQuotes = false;
    
    while (ss.good()) {
        char ch = ss.get();
        
        if (ch == EOF || ch == '\n') break;
        
        if (ch == '"') {
            inQuotes = !inQuotes; 
        } else if (ch == ',' && !inQuotes) {
            break; 
        } else {
            field += ch;
        }
    }
    return field;
}

// Function to insert data into the linked list
void insertNews(news*& head, string title, string text, string subject, string date) {
    news* newNode = new news{title, text, subject, date, nullptr}; 
    if (!head) {
        head = newNode;
        return;
    }
    news* temp = head;
    while (temp->address) temp = temp->address;
    temp->address = newNode;
}

// Function to load news from a CSV file into a linked list
void loadNews(string filename, news*& head) {
    fstream file(filename, ios::in);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    getline(file, line); // Skip the header
    while (getline(file, line)) {
        stringstream ss(line);
        string title = getCSVField(ss);
        string text = getCSVField(ss);
        string subject = getCSVField(ss);
        string date = getCSVField(ss);
        insertNews(head, title, text, subject, date);
    }
    file.close();
}

// Function to display news articles
void displayNews(news* head) {
    while (head) {
        cout << "Title: " << head->title << "\nText: " << head->text 
             << "\nSubject: " << head->subject << "\nDate: " << head->date << "\n\n";
        head = head->address;
    }
}

// Convert date string to comparable format (YYYYMMDD)
int convertDateToComparable(const string& date) {
    tm tm = {};
    istringstream ss(date);

    ss >> get_time(&tm, "%d-%b-%y"); // Example: "31-Dec-17"
    return (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
}

// Merge two sorted linked lists
news* merge(news* a, news* b) {
    if (!a) return b;
    if (!b) return a;

    news* result = nullptr;

    if (convertDateToComparable(a->date) <= convertDateToComparable(b->date)) {
        result = a;
        result->address = merge(a->address, b);
    } else {
        result = b;
        result->address = merge(a, b->address);
    }

    return result;
}

// Split linked list into two halves
void split(news* source, news** frontRef, news** backRef) {
    if (!source || !source->address) {
        *frontRef = source;
        *backRef = nullptr;
        return;
    }

    news* slow = source;
    news* fast = source->address;

    // Move fast two nodes and slow one node
    while (fast && fast->address) {
        slow = slow->address;
        fast = fast->address->address;
    }

    *frontRef = source;
    *backRef = slow->address;
    slow->address = nullptr; // Break the list into two
}

// Merge Sort for linked list
void mergeSort(news** headRef) {
    if (!headRef || !(*headRef) || !((*headRef)->address)) {
        return; // Base case: empty or single element list
    }

    news* head = *headRef;
    news* a;
    news* b;

    split(head, &a, &b); // Split list into halves

    mergeSort(&a); // Recursively sort each half
    mergeSort(&b);

    *headRef = merge(a, b); // Merge sorted halves
}

// Function to count the number of articles in a linked list
int countArticles(news* head) {
    int count = 0;
    while (head) {
        count++;
        head = head->address;
    }
    return count;
}

// Function to search articles by subject
void searchBySubject(news* head, string subject) {
    cout << "===== Articles with Subject: " << subject << " =====\n";
    bool found = false;
    while (head) {
        if (head->subject == subject) {
            cout << "Title: " << head->title << "\nText: " << head->text
                 << "\nSubject: " << head->subject << "\nDate: " << head->date << "\n\n";
            found = true;
        }

        head = head->address;
    }

    if (!found) {
        cout << "No articles found with subject: " << subject << "\n";
    }
}

void searchByYear(news* head, int year) {
    bool found = false;
    
    while (head) {
        tm tm = {}; 
        istringstream ss(head->date);

        ss >> get_time(&tm, "%d-%b-%y");

        if (ss.fail()) {
            head = head->address;
            continue;
        } 

        int articleYear = tm.tm_year + 1900; 
        if (articleYear == year) {
            cout << "Title: " << head->title << "\nText: " << head->text
                 << "\nSubject: " << head->subject << "\nDate: " << head->date << "\n\n";
            found = true;
        }

        head = head->address;
    }

    if (!found) {
        cout << "No articles found for year: " << year << "\n";
    }
}


// Function to analyze and report the percentage of fake political news articles for each month in 2016
void analyzeFakeNews(news* fakeNewsHead) {
    int fakeNewsCount[12] = {0}; 
    int totalFakeNewsCount[12] = {0}; 

    news* current = fakeNewsHead;
    while (current) {

        tm tm = {};
        istringstream ss(current->date);
        ss >> get_time(&tm, "%d-%b-%y");

        if (ss.fail()) {
            current = current->address;
            continue;
        }

        int year = tm.tm_year + 1900;
        int month = tm.tm_mon; 
        // Only process news from 2016
        if (year == 2016) {
            totalFakeNewsCount[month]++; // Count all fake news that year

            // Convert subject to lowercase for case-insensitive comparison
            string subjectLower = current->subject;
            for (char& c : subjectLower) c = tolower(c);

            if (subjectLower.find("politics") != string::npos) {
                fakeNewsCount[month]++; // Count political fake news
            }
        }

        current = current->address;
    }

    // Display results
    cout << "Percentage of Fake Political News Articles in 2016\n";
    cout << "=================================================\n";

    string monthNames[12] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    for (int i = 0; i < 12; i++) {
        float percentage = 0.0f;
        if (totalFakeNewsCount[i] > 0) {
            percentage = (static_cast<float>(fakeNewsCount[i]) / totalFakeNewsCount[i]) * 100;
        }

        cout << monthNames[i] << " | ";
        for (int j = 0; j < static_cast<int>(percentage); j++) {
            cout << "*";
        }

        cout << " | " << fixed << setprecision(1) << percentage << "%\n";
    }
}

int main() {

    int choice;
    string searchSub;
    int yearToSearch;

    news* trueNewsHead = nullptr;
    news* fakeNewsHead = nullptr;
    news* cleanNewsHead = nullptr;

    loadNews("D:\\new dstr assignment\\true (2).csv", trueNewsHead);
    loadNews("D:\\new dstr assignment\\fake (2).csv", fakeNewsHead);
    loadNews("D:\\new dstr assignment\\clean.csv", cleanNewsHead);
    while (true) {
        cout << "\nWelcome, please select the function you want" << endl;
        cout << "1. Display the dataset" << endl;
        cout << "2. Sort the dataset by date in ascending order" << endl;
        cout << "3. Calculate the total number of articles in dataset" << endl;
        cout << "4. Search articles in dataset" << endl;
        cout << "5. Report the percentage of fake political news articles for each month in 2016" << endl;
        cout << "6. Exit" << endl;
        cout << "Your choice: ";
        cin >> choice;

        if (choice == 6) {
            cout << "Exiting the program..." << endl;
            break;
        }

        switch (choice) {
            case 1:
                cout << "Which dataset you want to display?" << endl;
                cout << "1. True.csv" << endl;
                cout << "2. Fake.csv" << endl;
                cout << "3. Both datasets" << endl;
                cout << "Dataset: ";
                cin >> choice;

                if (choice == 1) {
                    auto start = high_resolution_clock::now();
                    cout << "===== True News Articles =====\n";
                    displayNews(trueNewsHead);
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() << "ms" << endl;
                    
                } else if (choice == 2) {
                    auto start = high_resolution_clock::now();
                    cout << "===== Fake News Articles =====\n";
                    displayNews(fakeNewsHead);
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() << "ms" << endl;
                } else if (choice == 3) {
                    auto start = high_resolution_clock::now();
                    cout << "===== Both News Articles =====\n";
                    displayNews(cleanNewsHead);
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() << "ms" << endl;
                } else {
                    cerr << "Invalid Input!" << endl;
                }
                break;

            case 2:
                cout << "Which dataset you want to sort?" << endl;
                cout << "1. True.csv" << endl;
                cout << "2. Fake.csv" << endl;
                cout << "3. Both datasets" << endl;
                cout << "Dataset: ";
                cin >> choice;

                if (choice == 1) {
                    auto start = high_resolution_clock::now();
                    mergeSort(&trueNewsHead);
                    cout << "===== True News Articles (After Sorting) =====\n";
                    displayNews(trueNewsHead);
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() <<"ms" << endl;
                } else if (choice == 2) {
                    auto start = high_resolution_clock::now();
                    mergeSort(&fakeNewsHead);
                    cout << "===== Fake News Articles (After Sorting) =====\n";
                    displayNews(fakeNewsHead);
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() <<"ms" << endl;
                } else if (choice == 3) {
                    auto start = high_resolution_clock::now();
                    mergeSort(&cleanNewsHead);
                    cout << "===== Both News Articles (After Sorting) =====\n";
                    displayNews(cleanNewsHead);
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() <<"ms" << endl;
                } else {
                    cerr << "Invalid Input!" << endl;
                }
                break;

            case 3:
                cout << "Which dataset you want to calculate?" << endl;
                cout << "1. True.csv" << endl;
                cout << "2. Fake.csv" << endl;
                cout << "3. Both datasets" << endl;
                cout << "Dataset: ";
                cin >> choice;

                if (choice == 1) {
                    auto start = high_resolution_clock::now();
                    cout << "Total number of true news articles: " << countArticles(trueNewsHead) << endl;
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() <<"ms" << endl;
                } else if (choice == 2) {
                    auto start = high_resolution_clock::now();
                    cout << "Total number of fake news articles: " << countArticles(fakeNewsHead) << endl;
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() <<"ms" << endl;
                } else if (choice == 3) {
                    auto start = high_resolution_clock::now();
                    cout << "Total number of articles in both datasets: " << countArticles(trueNewsHead) + countArticles(fakeNewsHead) << endl;
                    auto stop = high_resolution_clock::now();
                    printMemoryUsage(); 
                    auto duration = duration_cast<milliseconds>(stop - start);
                    cout << "Execution Time: " << duration.count() <<"ms" << endl;
                } else {
                    cerr << "Invalid Input!" << endl;
                }
                break;

            case 4:
                cout << "Which dataset you want to search?" << endl;
                cout << "1. True.csv" << endl;
                cout << "2. Fake.csv" << endl;
                cout << "3. Both datasets" << endl;
                cout << "Dataset: ";
                cin >> choice;

                cout << "1. Search by subject" << endl;
                cout << "2. Search by year" << endl;
                cout << "Search by: ";
                cin >> choice;

                if (choice == 1) {
                    cout << "Subject: ";
                    cin >> searchSub;

                    if (choice == 1) {
                        auto start = high_resolution_clock::now();
                        searchBySubject(trueNewsHead, searchSub);
                        auto stop = high_resolution_clock::now();
                        printMemoryUsage(); 
                        auto duration = duration_cast<milliseconds>(stop - start);
                        cout << "Execution Time: " << duration.count() <<"ms" << endl;
                    }
                    else if (choice == 2) {
                        auto start = high_resolution_clock::now();
                        searchBySubject(fakeNewsHead, searchSub);
                        auto stop = high_resolution_clock::now();
                        printMemoryUsage(); 
                        auto duration = duration_cast<milliseconds>(stop - start);
                        cout << "Execution Time: " << duration.count() <<"ms" << endl;
                    } else{
                        auto start = high_resolution_clock::now();
                        searchBySubject(cleanNewsHead, searchSub);
                        auto stop = high_resolution_clock::now();
                        printMemoryUsage(); 
                        auto duration = duration_cast<milliseconds>(stop - start);
                        cout << "Execution Time: " << duration.count() <<"ms" << endl;
                    }
                } else if (choice == 2) {
                    cout << "Year: ";
                    cin >> yearToSearch;

                    if (choice == 1){
                        auto start = high_resolution_clock::now();
                        searchByYear(trueNewsHead, yearToSearch);
                        auto stop = high_resolution_clock::now();
                        printMemoryUsage(); 
                        auto duration = duration_cast<milliseconds>(stop - start);
                        cout << "Execution Time: " << duration.count() <<"ms" << endl;
                    } else if (choice == 2) {
                        auto start = high_resolution_clock::now();
                        searchByYear(fakeNewsHead, yearToSearch);
                        auto stop = high_resolution_clock::now();
                        printMemoryUsage(); 
                        auto duration = duration_cast<milliseconds>(stop - start);
                        cout << "Execution Time: " << duration.count() <<"ms" << endl;
                    }
                    else{
                        auto start = high_resolution_clock::now();
                        searchByYear(cleanNewsHead, yearToSearch);
                        auto stop = high_resolution_clock::now();
                        printMemoryUsage(); 
                        auto duration = duration_cast<milliseconds>(stop - start);
                        cout << "Execution Time: " << duration.count() <<"ms" << endl;
                    }
                } else {
                    cerr << "Invalid Input!" << endl;
                }
                break;

            case 5:
                analyzeFakeNews(fakeNewsHead);
                break;

            default:
                cerr << "Invalid choice! Please try again." << endl;
                break;
        }
    }

    return 0;
}
