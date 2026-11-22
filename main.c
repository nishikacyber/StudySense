
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SUBJECTS 100
#define MAX_TASKS 200
#define MAX_ACTIVITY 1000

#define RESET   "\033[0m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"

typedef struct {
    int day, month, year;
} Date;

typedef struct {
    int id;
    char name[50];
    Date examDate;
} Subject;

typedef struct {
    int id;
    int subjectId;
    char topic[100];
    Date deadline;
    int duration;
    int completed;
} Task;

typedef struct {
    int dateKey;
    int completedTasks;
} Activity;

Subject subjects[MAX_SUBJECTS];
Task tasks[MAX_TASKS];
Activity activity[MAX_ACTIVITY];

int subjectCount = 0;
int taskCount = 0;
int activityCount = 0;

/* FUNCTION PROTOTYPES */

void displayDashboard(void);
void displayMenu(void);
void addSubject(void);
void addTask(void);
void viewTasks(void);
void markCompleted(void);
void viewProgress(void);
void viewStreak(void);
void examCountdown(void);
void saveData(void);
void loadData(void);

int readInt(char message[]);
void readString(char message[], char output[], int size);
int validDate(Date d);
int dateKey(Date d);
Date todayDate(void);
int daysBetween(Date d1, Date d2);
int findSubject(int id);
int hasActivity(int key);
void recordActivity(void);
int getCurrentStreak(void);

/* INPUT FUNCTIONS */

int readInt(char message[]) {
    char input[100];
    int value;

    while (1) {
        printf("%s", message);

        if (fgets(input, sizeof(input), stdin) == NULL)
            return -1;

        if (sscanf(input, "%d", &value) == 1)
            return value;

        printf(RED "Invalid input! Enter a number.\n" RESET);
    }
}

void readString(char message[], char output[], int size) {
    printf("%s", message);

    if (fgets(output, size, stdin) == NULL) {
        output[0] = '\0';
        return;
    }

    output[strcspn(output, "\n")] = '\0';
}

/* DATE FUNCTIONS */

int validDate(Date d) {
    if (d.year < 1900 || d.month < 1 || d.month > 12)
        return 0;

    int days[] = {31, 28, 31, 30, 31, 30,
                  31, 31, 30, 31, 30, 31};

    if ((d.year % 400 == 0) ||
        (d.year % 4 == 0 && d.year % 100 != 0))
        days[1] = 29;

    return d.day >= 1 && d.day <= days[d.month - 1];
}

int dateKey(Date d) {
    return d.year * 10000 + d.month * 100 + d.day;
}

Date todayDate(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    Date d = {t->tm_mday, t->tm_mon + 1, t->tm_year + 1900};
    return d;
}

int daysBetween(Date d1, Date d2) {
    struct tm a = {0}, b = {0};

    a.tm_mday = d1.day;
    a.tm_mon = d1.month - 1;
    a.tm_year = d1.year - 1900;

    b.tm_mday = d2.day;
    b.tm_mon = d2.month - 1;
    b.tm_year = d2.year - 1900;

    time_t t1 = mktime(&a);
    time_t t2 = mktime(&b);

    return (int)(difftime(t2, t1) / 86400);
}

/* SUBJECT SEARCH */

int findSubject(int id) {
    for (int i = 0; i < subjectCount; i++) {
        if (subjects[i].id == id)
            return i;
    }

    return -1;
}

/* ACTIVITY TRACKER */

int hasActivity(int key) {
    for (int i = 0; i < activityCount; i++) {
        if (activity[i].dateKey == key)
            return i;
    }

    return -1;
}

void recordActivity(void) {
    int key = dateKey(todayDate());
    int index = hasActivity(key);

    if (index != -1) {
        activity[index].completedTasks++;
    }
    else if (activityCount < MAX_ACTIVITY) {
        activity[activityCount].dateKey = key;
        activity[activityCount].completedTasks = 1;
        activityCount++;
    }
}

/* FILE HANDLING */

void saveData(void) {
    FILE *fp;

    fp = fopen("subjects.dat", "wb");
    if (fp != NULL) {
        fwrite(&subjectCount, sizeof(int), 1, fp);
        fwrite(subjects, sizeof(Subject), subjectCount, fp);
        fclose(fp);
    }

    fp = fopen("tasks.dat", "wb");
    if (fp != NULL) {
        fwrite(&taskCount, sizeof(int), 1, fp);
        fwrite(tasks, sizeof(Task), taskCount, fp);
        fclose(fp);
    }

    fp = fopen("activity.dat", "wb");
    if (fp != NULL) {
        fwrite(&activityCount, sizeof(int), 1, fp);
        fwrite(activity, sizeof(Activity), activityCount, fp);
        fclose(fp);
    }
}

void loadData(void) {
    FILE *fp;
    int count;

    fp = fopen("subjects.dat", "rb");
    if (fp != NULL) {
        if (fread(&count, sizeof(int), 1, fp) == 1 &&
            count >= 0 && count <= MAX_SUBJECTS) {
            subjectCount = (int)fread(
                subjects, sizeof(Subject), count, fp);
        }
        fclose(fp);
    }

    fp = fopen("tasks.dat", "rb");
    if (fp != NULL) {
        if (fread(&count, sizeof(int), 1, fp) == 1 &&
            count >= 0 && count <= MAX_TASKS) {
            taskCount = (int)fread(
                tasks, sizeof(Task), count, fp);
        }
        fclose(fp);
    }

    fp = fopen("activity.dat", "rb");
    if (fp != NULL) {
        if (fread(&count, sizeof(int), 1, fp) == 1 &&
            count >= 0 && count <= MAX_ACTIVITY) {
            activityCount = (int)fread(
                activity, sizeof(Activity), count, fp);
        }
        fclose(fp);
    }
}

/* ADD SUBJECT */

void addSubject(void) {
    if (subjectCount >= MAX_SUBJECTS) {
        printf(RED "\nSubject limit reached!\n" RESET);
        return;
    }

    Subject s;

    s.id = subjectCount + 1;

    printf(CYAN BOLD "\n========== ADD NEW SUBJECT ==========\n" RESET);

    readString("Enter subject name: ", s.name, sizeof(s.name));

    if (strlen(s.name) == 0) {
        printf(RED "Subject name cannot be empty!\n" RESET);
        return;
    }

    printf("Enter exam date (DD MM YYYY): ");

    if (scanf("%d %d %d",
        &s.examDate.day, &s.examDate.month,
        &s.examDate.year) != 3) {
        printf(RED "Invalid date input!\n" RESET);
        while (getchar() != '\n');
        return;
    }

    while (getchar() != '\n');

    if (!validDate(s.examDate)) {
        printf(RED "Invalid date!\n" RESET);
        return;
    }

    subjects[subjectCount++] = s;

    saveData();

    printf(GREEN "\nSubject added successfully!\n" RESET);
    printf("Subject ID: %d\n", s.id);
}

/* ADD TASK */

void addTask(void) {
    if (taskCount >= MAX_TASKS) {
        printf(RED "\nTask limit reached!\n" RESET);
        return;
    }

    if (subjectCount == 0) {
        printf(RED "\nAdd a subject first!\n" RESET);
        return;
    }

    Task t;

    printf(CYAN BOLD "\n========== ADD STUDY TASK ==========\n" RESET);

    printf("\nAvailable Subjects:\n");

    for (int i = 0; i < subjectCount; i++)
        printf("%d. %s\n", subjects[i].id, subjects[i].name);

    int id = readInt("\nEnter Subject ID: ");

    if (findSubject(id) == -1) {
        printf(RED "Subject not found!\n" RESET);
        return;
    }

    t.subjectId = id;

    readString("Enter topic name: ", t.topic, sizeof(t.topic));

    if (strlen(t.topic) == 0) {
        printf(RED "Topic cannot be empty!\n" RESET);
        return;
    }

    printf("Enter deadline (DD MM YYYY): ");

    if (scanf("%d %d %d",
        &t.deadline.day, &t.deadline.month,
        &t.deadline.year) != 3) {
        printf(RED "Invalid date!\n" RESET);
        while (getchar() != '\n');
        return;
    }

    while (getchar() != '\n');

    if (!validDate(t.deadline)) {
        printf(RED "Invalid deadline!\n" RESET);
        return;
    }

    t.duration = readInt("Enter study duration (hours): ");

    if (t.duration <= 0) {
        printf(RED "Invalid duration!\n" RESET);
        return;
    }

    t.id = taskCount + 1;
    t.completed = 0;

    tasks[taskCount++] = t;

    saveData();

    printf(GREEN "\nTask added successfully!\n" RESET);
    printf("Task ID: %d\n", t.id);
}

/* VIEW TASKS */

void viewTasks(void) {
    printf(CYAN BOLD "\n========== ALL STUDY TASKS ==========\n" RESET);

    if (taskCount == 0) {
        printf(YELLOW "No tasks found!\n" RESET);
        return;
    }

    for (int i = 0; i < taskCount; i++) {
        int s = findSubject(tasks[i].subjectId);

        printf("\n------------------------------------------\n");
        printf("Task ID   : %d\n", tasks[i].id);
        printf("Subject   : %s\n",
               s == -1 ? "Unknown" : subjects[s].name);
        printf("Topic     : %s\n", tasks[i].topic);

        printf("Deadline  : %02d-%02d-%04d\n",
               tasks[i].deadline.day,
               tasks[i].deadline.month,
               tasks[i].deadline.year);

        printf("Duration  : %d hours\n", tasks[i].duration);

        printf("Status    : %s\n",
               tasks[i].completed ? "Completed" : "Pending");
    }

    printf("\nTotal Tasks: %d\n", taskCount);
}

/* MARK TASK COMPLETED */

void markCompleted(void) {
    if (taskCount == 0) {
        printf(YELLOW "\nNo tasks available!\n" RESET);
        return;
    }

    viewTasks();

    int id = readInt("\nEnter Task ID to complete: ");

    for (int i = 0; i < taskCount; i++) {
        if (tasks[i].id == id) {

            if (tasks[i].completed) {
                printf(YELLOW "Task already completed!\n" RESET);
                return;
            }

            tasks[i].completed = 1;
            recordActivity();
            saveData();

            printf(GREEN "\nTask marked as completed!\n" RESET);
            return;
        }
    }

    printf(RED "\nInvalid Task ID!\n" RESET);
}

/* STUDY PROGRESS */

void viewProgress(void) {
    int completed = 0;
    int pending = 0;

    for (int i = 0; i < taskCount; i++) {
        if (tasks[i].completed)
            completed++;
        else
            pending++;
    }

    float progress = taskCount > 0
        ? (completed * 100.0f) / taskCount : 0;

    printf(CYAN BOLD "\n========== STUDY PROGRESS ==========\n" RESET);

    printf("\nTotal Tasks     : %d\n", taskCount);
    printf("Completed Tasks : %d\n", completed);
    printf("Pending Tasks   : %d\n", pending);
    printf("Study Progress  : %.2f%%\n", progress);

    printf("\nProgress: [");

    int bars = (int)(progress / 5);

    for (int i = 0; i < 20; i++)
        printf("%s", i < bars ? "#" : "-");

    printf("]\n");
}

/* STUDY STREAK */

void viewStreak(void) {
    Date today = todayDate();
    int key = dateKey(today);

    int streak = 0;

    if (hasActivity(key) == -1) {
        today = (Date){today.day, today.month, today.year};

        struct tm t = {0};
        t.tm_mday = today.day - 1;
        t.tm_mon = today.month - 1;
        t.tm_year = today.year - 1900;
        mktime(&t);

        today.day = t.tm_mday;
        today.month = t.tm_mon + 1;
        today.year = t.tm_year + 1900;
    }

    while (hasActivity(dateKey(today)) != -1) {
        streak++;

        struct tm t = {0};
        t.tm_mday = today.day - 1;
        t.tm_mon = today.month - 1;
        t.tm_year = today.year - 1900;
        mktime(&t);

        today.day = t.tm_mday;
        today.month = t.tm_mon + 1;
        today.year = t.tm_year + 1900;
    }

    printf(CYAN BOLD "\n========== STUDY STREAK ==========\n" RESET);
    printf("\nCurrent Study Streak: %d days\n", streak);

    if (streak > 0)
        printf(GREEN "Keep up the consistency!\n" RESET);
    else
        printf(YELLOW "Complete a task today to start your streak!\n" RESET);
}

/* EXAM COUNTDOWN */

void examCountdown(void) {
    if (subjectCount == 0) {
        printf(YELLOW "\nNo subjects available!\n" RESET);
        return;
    }

    Date today = todayDate();
    int found = 0;
    int minimum = 0;
    int index = -1;

    printf(CYAN BOLD "\n========== EXAM COUNTDOWN ==========\n" RESET);

    for (int i = 0; i < subjectCount; i++) {
        int remaining = daysBetween(today, subjects[i].examDate);

        printf("\n%s: %02d-%02d-%04d\n",
               subjects[i].name,
               subjects[i].examDate.day,
               subjects[i].examDate.month,
               subjects[i].examDate.year);

        if (remaining < 0) {
            printf("Exam date has passed.\n");
        }
        else {
            printf("Days remaining: %d\n", remaining);

            if (!found || remaining < minimum) {
                minimum = remaining;
                index = i;
                found = 1;
            }
        }
    }

    if (found) {
        printf(GREEN "\nNext upcoming exam: %s\n" RESET,
               subjects[index].name);
        printf("Only %d days remaining!\n", minimum);
    }
    else {
        printf(YELLOW "\nNo upcoming exams!\n" RESET);
    }
}

/* DASHBOARD */


void displayDashboard(void) {
    int completed = 0;
    int pending;
    float progress;
    Date today = todayDate();

    for (int i = 0; i < taskCount; i++) {
        if (tasks[i].completed)
            completed++;
    }

    pending = taskCount - completed;
    progress = taskCount > 0
        ? (completed * 100.0f) / taskCount : 0;

    int bars = (int)(progress / 100.0f * 30);

    int nearest = -1;
    int minDays = 0;

    for (int i = 0; i < subjectCount; i++) {
        int remaining = daysBetween(today, subjects[i].examDate);

        if (remaining >= 0 &&
            (nearest == -1 || remaining < minDays)) {
            nearest = i;
            minDays = remaining;
        }
    }

    printf(CYAN BOLD "\n");
    printf("  +----------------------------------------------------------+\n");
    printf("  |                                                          |\n");
    printf("  |                    STUDYSENSE                            |\n");
    printf("  |              SMART STUDY PLANNER                         |\n");
    printf("  |                                                          |\n");
    printf("  +----------------------------------------------------------+\n" RESET);

    printf(YELLOW "\n  Welcome back! Ready to achieve your goals?\n" RESET);

    printf(CYAN "\n  TODAY: %02d / %02d / %04d\n" RESET,
           today.day, today.month, today.year);

    printf(MAGENTA BOLD "\n  +-------------------- YOUR OVERVIEW --------------------+\n" RESET);

    printf("\n  |  SUBJECTS             : %-25d |\n", subjectCount);
    printf("  |  TOTAL STUDY TASKS    : %-25d |\n", taskCount);
    printf(GREEN "  |  COMPLETED TASKS      : %-25d |\n" RESET, completed);
    printf(YELLOW "  |  PENDING TASKS        : %-25d |\n" RESET, pending);

    printf(MAGENTA BOLD "\n  +-------------------- STUDY PROGRESS -------------------+\n" RESET);

    printf("\n  Progress: %.1f%%\n\n", progress);

    printf("  [");
    for (int i = 0; i < 30; i++) {
        if (i < bars)
            printf(GREEN "#" RESET);
        else
            printf(CYAN "-" RESET);
    }
    printf("]\n");

    printf(MAGENTA BOLD "\n  +------------------- DAILY MOTIVATION ------------------+\n" RESET);

    printf(YELLOW "\n  CURRENT STUDY STREAK : %d days\n" RESET,
           getCurrentStreak());

    if (progress == 100 && taskCount > 0)
        printf(GREEN "\n  Excellent! All tasks completed!\n" RESET);
    else if (completed > 0)
        printf(GREEN "\n  Great work! Keep going!\n" RESET);
    else
        printf(YELLOW "\n  Every small step counts. Start today!\n" RESET);

    printf(MAGENTA BOLD "\n  +-------------------- UPCOMING EXAM --------------------+\n" RESET);

    if (nearest != -1) {
        printf("\n  Subject  : %s\n", subjects[nearest].name);

        printf("  Exam Date: %02d-%02d-%04d\n",
               subjects[nearest].examDate.day,
               subjects[nearest].examDate.month,
               subjects[nearest].examDate.year);

        printf(RED BOLD "  Days Left: %d\n" RESET, minDays);
    }
    else {
        printf(YELLOW "\n  No upcoming exams scheduled.\n" RESET);
    }

    printf(CYAN BOLD "\n  +----------------------------------------------------------+\n" RESET);
}

   
/* MAIN MENU */


void displayMenu(void) {

    printf(MAGENTA BOLD "\n");
    printf("  +----------------------------------------------------------+\n");
    printf("  |                       MAIN MENU                          |\n");
    printf("  +----------------------------------------------------------+\n" RESET);

    printf(CYAN BOLD "\n       [1] " RESET " Add New Subject");
    printf(CYAN BOLD "          [2] " RESET " Add Study Task\n");

    printf(CYAN BOLD "       [3] " RESET " View All Tasks");
    printf(CYAN BOLD "           [4] " RESET " Complete Task\n");

    printf(CYAN BOLD "       [5] " RESET " Study Progress");
    printf(CYAN BOLD "           [6] " RESET " Study Streak\n");

    printf(CYAN BOLD "       [7] " RESET " Exam Countdown");
    printf(RED BOLD "           [8] " RESET " Exit\n");

    printf(MAGENTA BOLD "\n  +----------------------------------------------------------+\n" RESET);
}


/* CALCULATE CURRENT STUDY STREAK */

int getCurrentStreak(void) {

    time_t now = time(NULL);
    struct tm current = *localtime(&now);

    int streak = 0;

    // If no task completed today, check from yesterday
    if (hasActivity(dateKey(todayDate())) == -1) {
        current.tm_mday--;
        mktime(&current);
    }

    while (1) {

        Date d = {
            current.tm_mday,
            current.tm_mon + 1,
            current.tm_year + 1900
        };

        if (hasActivity(dateKey(d)) == -1)
            break;

        streak++;

        current.tm_mday--;
        mktime(&current);
    }

    return streak;
}


/* MAIN FUNCTION */

int main(void) {
    loadData();

    int choice;

    while (1) {
        system("cls");

    displayDashboard();
    displayMenu();

        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                addSubject();
                break;

            case 2:
                addTask();
                break;

            case 3:
                viewTasks();
                break;

            case 4:
                markCompleted();
                break;

            case 5:
                viewProgress();
                break;

            case 6:
                viewStreak();
                break;

            case 7:
                examCountdown();
                break;

            case 8:
                saveData();
                printf(YELLOW "\nThank you for using StudySense!\n" RESET);
                return 0;

            default:
                printf(RED "\nInvalid choice! Enter 1-8.\n" RESET);
        }

        printf("\nPress Enter to continue...");
        getchar();
    }

    return 0;
}