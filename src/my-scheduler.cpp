#include <mesos/mesos.hpp>
#include <mesos/scheduler.hpp>
#include <mesos/resources.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <memory>
#include <vector>

using std::cout;
using std::clog;
using std::endl;
using std::string;
using std::vector;
using boost::lexical_cast;
using namespace mesos;

const int CPUS_PER_TASK = 1;
const int MEM_PER_TASK = 64;

class MyScheduler : public Scheduler {
public:
    MyScheduler(ExecutorInfo& executor, const std::string& role);
    ~MyScheduler();
    void registered(SchedulerDriver* driver, const FrameworkID& frameworkId, const MasterInfo& masterInfo) override;
    void reregistered(SchedulerDriver* driver, const MasterInfo& masterInfo) override;
    void disconnected(SchedulerDriver* driver) override;
    void resourceOffers(SchedulerDriver* driver, const std::vector<Offer>& offers) override;
    void offerRescinded(SchedulerDriver* driver, const OfferID& offerId) override;
    void statusUpdate(SchedulerDriver* driver, const TaskStatus& status) override;
    void frameworkMessage(SchedulerDriver* driver, const ExecutorID& executorId, const SlaveID& slaveId, const std::string& data) override;
    void slaveLost(SchedulerDriver* driver, const SlaveID& slaveId) override;
    void executorLost(SchedulerDriver* driver, const ExecutorID& executorId, const SlaveID& slaveId, int status) override;
    void error(SchedulerDriver* driver, const std::string& message) override;

private:
    ExecutorInfo executor_;
    string role_;
    int tasksLaunched_;
};

// {{{ MyScheduler impl
MyScheduler::MyScheduler(ExecutorInfo& executor, const std::string& role) :
    executor_(executor),
    role_(role),
    tasksLaunched_(0)
{
}

MyScheduler::~MyScheduler()
{
}

void MyScheduler::registered(SchedulerDriver* driver, const FrameworkID& frameworkId, const MasterInfo& masterInfo)
{
    clog << "Registered!" << endl;
}

void MyScheduler::reregistered(SchedulerDriver* driver, const MasterInfo& masterInfo)
{
    clog << "Reregistered!" << endl;
}

void MyScheduler::disconnected(SchedulerDriver* driver)
{
    clog << "Disconnected!" << endl;
}

void MyScheduler::resourceOffers(SchedulerDriver* driver, const std::vector<Offer>& offers)
{
    clog << "Resource offers received" << endl;

    static const Resources TASK_RESOURCES = Resources::parse(
        "cpus:" + stringify(CPUS_PER_TASK) +
        ";mem:" + stringify(MEM_PER_TASK)).get();

    int totalTasks = 0xFFFF;

    for (size_t i = 0; i < offers.size(); i++) {
        const Offer& offer = offers[i];
        vector<TaskInfo> tasks;
        Resources remaining = offer.resources();

        while (tasksLaunched_ < totalTasks && TASK_RESOURCES <= remaining.flatten()) {
            int taskId = tasksLaunched_++;

            clog << "Starting task " << taskId << " on " << offer.hostname() << endl;

            // create a task
            TaskInfo task;
            task.set_name("Task " + lexical_cast<string>(taskId));
            task.mutable_task_id()->set_value(lexical_cast<string>(taskId));
            task.mutable_slave_id()->MergeFrom(offer.slave_id());
            task.mutable_executor()->MergeFrom(executor_);

            // assigning resources this task may use
            Option<Resources> resources = remaining.find(TASK_RESOURCES, role_);
            task.mutable_resources()->MergeFrom(resources.get());
            remaining -= resources.get();

            tasks.push_back(task);
        }

        driver->launchTasks(offer.id(), tasks);
    }
}

void MyScheduler::offerRescinded(SchedulerDriver* driver, const OfferID& offerId)
{
    clog << "Offer " << offerId.value() << " rescinded!" << endl;
}

void MyScheduler::statusUpdate(SchedulerDriver* driver, const TaskStatus& status)
{
    clog << "statusUpdate for task " << status.task_id().value() << ": " << status.message() << endl;
}

void MyScheduler::frameworkMessage(SchedulerDriver* driver, const ExecutorID& executorId, const SlaveID& slaveId, const std::string& data)
{
    clog << "frameworkMessage!" << endl;
}

void MyScheduler::slaveLost(SchedulerDriver* driver, const SlaveID& slaveId)
{
    clog << "Slave " << slaveId.value() << " lost!" << endl;
}

void MyScheduler::executorLost(SchedulerDriver* driver, const ExecutorID& executorId, const SlaveID& slaveId, int status)
{
    clog << "Executor " << executorId.value() << " lost! status=" << status << endl;
}

void MyScheduler::error(SchedulerDriver* driver, const std::string& message)
{
    clog << "error! " << message << endl;
}
// }}}

int main(int argc, const char* argv[])
{
    string role = "*";
    std::string master = "192.168.122.120:5050"; // "127.0.0.1:5050";

    ExecutorInfo executor;
    executor.mutable_executor_id()->set_value("default");
    executor.mutable_command()->set_value("/home/trapni/my-executor");
    executor.set_name("My Test Executor (C++)");
    executor.set_source("my_test_source");

    std::unique_ptr<Scheduler> scheduler(new MyScheduler(executor, role));

    FrameworkInfo framework;
    framework.set_user(""); // Have Mesos fill in the current user.
    framework.set_name("Test Framework (C++)");
    framework.set_role(role);

    std::unique_ptr<MesosSchedulerDriver> driver(new MesosSchedulerDriver(scheduler.get(), framework, master));

    int status = driver->run();

    return status == mesos::DRIVER_STOPPED ? 0 : 1;
}
