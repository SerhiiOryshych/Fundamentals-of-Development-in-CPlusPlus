#include<map>
#include<tuple>
#include<vector>
#include<iostream>
#include<algorithm>

using namespace std;

// Enumerated type for task status.
enum class TaskStatus {
    NEW,
    IN_PROGRESS,
    TESTING,
    DONE
};

// Declaring a type synonym for map<TaskStatus, int>,
// allowing you to store the count of tasks for each status.
using TasksInfo = map<TaskStatus, int>;

class TeamTasks {
public:
    // Get statistics on task statuses for a specific developer.
    [[nodiscard]] TasksInfo GetPersonTasksInfo(const string &person) const {
        TasksInfo tasksInfo;

        if (teamTasks.count(person) == 0) {
            return tasksInfo;
        }

        for (auto &taskStatus: teamTasks.at(person)) {
            tasksInfo[taskStatus]++;
        }

        return tasksInfo;
    }

    // Add a new task (in the NEW status) for a specific developer.
    void AddNewTask(const string &person) {
        teamTasks[person].push_back(TaskStatus::NEW);
    }

    // Update the statuses for a specific developer for the given number of tasks, details below.
    tuple<TasksInfo, TasksInfo> PerformPersonTasks(const string &person, int task_count) {
        auto personTasks = teamTasks[person];
        sort(personTasks.begin(), personTasks.end());

        vector<TaskStatus> updatedPersonTasks;
        TasksInfo noChangeTasksInfo, updatedTasksInfo;
        for (auto &taskStatus: personTasks) {
            auto newStatus = taskStatus;
            if (taskStatus != TaskStatus::DONE) {
                if (task_count > 0) {
                    newStatus = static_cast<TaskStatus>(static_cast<int>(taskStatus) + 1);
                    task_count--;

                    updatedTasksInfo[newStatus]++;
                } else {
                    noChangeTasksInfo[taskStatus]++;
                }
            }

            updatedPersonTasks.push_back(newStatus);
        }
        teamTasks[person] = updatedPersonTasks;

        return {updatedTasksInfo, noChangeTasksInfo};
    }

private:
    map<string, vector<TaskStatus>> teamTasks;
};

// Accept a dictionary by value to be able to access missing keys using []
// and receive 0 without modifying the original dictionary.
void PrintTasksInfo(TasksInfo tasks_info) {
    cout << tasks_info[TaskStatus::NEW] << " new tasks" <<
         ", " << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress" <<
         ", " << tasks_info[TaskStatus::TESTING] << " tasks are being tested" <<
         ", " << tasks_info[TaskStatus::DONE] << " tasks are done" << endl;
}

int main() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia");
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << "Ilia's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
    cout << "Ivan's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

    TasksInfo updated_tasks, untouched_tasks;

    tie(updated_tasks, untouched_tasks) =
            tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    tie(updated_tasks, untouched_tasks) =
            tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    return 0;
}
