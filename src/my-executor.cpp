#include <mesos/executor.hpp>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

using std::cout;
using std::clog;
using std::endl;
using std::string;
using namespace mesos;

struct _threaded {
    pthread_t tid;
    std::function<void()> run;

    explicit _threaded(std::function<void()> fn) : tid(), run(fn) {}

    static void* main(void* pself) {
        _threaded* self = (_threaded*) pself;
        self->run();
        delete self;
        return nullptr;
    }
};

void threaded(std::function<void()> fn)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    _threaded* thr = new _threaded(fn);
    pthread_create(&thr->tid, &attr, &_threaded::main, thr);
}

class TestExecutor : public Executor {
public:
    TestExecutor()
    {
    }

    ~TestExecutor()
    {
    }

    void registered(ExecutorDriver* driver,
        const ExecutorInfo& executorInfo,
        const FrameworkInfo& frameworkInfo,
        const SlaveInfo& slaveInfo) override
    {
        clog << "Registered executor on " << slaveInfo.hostname() << endl;
    }

    void reregistered(ExecutorDriver* driver, const SlaveInfo& slaveInfo) override
    {
        clog << "Re-registered executor on " << slaveInfo.hostname() << endl;
    }

    void disconnected(ExecutorDriver* driver) override
    {
        clog << "Disconnected." << endl;
    }

    void launchTask(ExecutorDriver* driver, const TaskInfo& task) override
    {
        threaded([=]() {
            clog << "Starting task " << task.task_id().value() << endl;

            TaskStatus status;
            status.mutable_task_id()->MergeFrom(task.task_id());

            status.set_state(TASK_RUNNING);
            driver->sendStatusUpdate(status);

            clog << "task data: '" << task.data() << "'" << endl;

            // This is where one would perform the requested task.
            sleep(5);
            clog << "Finishing task " << task.task_id().value() << endl;

            status.set_state(TASK_FINISHED);
            driver->sendStatusUpdate(status);
        });
    }

    void killTask(ExecutorDriver* driver, const TaskID& taskId) override {
        clog << "kill task " << taskId.value() << endl;
    }

    void frameworkMessage(ExecutorDriver* driver, const string& data) override {
        clog << "Framework Message: '" << data << "'" << endl;
    }

    void shutdown(ExecutorDriver* driver) override {
        clog << "Shutdown." << endl;
    }

    void error(ExecutorDriver* driver, const string& message) override {
        clog << "error: " << message << endl;
    }
};

int main(int argc, const char* argv[])
{
    TestExecutor executor;
    MesosExecutorDriver driver(&executor);
    return driver.run() == DRIVER_STOPPED ? 0 : 1;
}
