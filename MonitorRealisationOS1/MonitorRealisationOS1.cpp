#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

template <typename T>
class Monitor {
private:
    T resource;
    bool isResourceAvailable;
    std::mutex mutex;
    std::condition_variable condition;

public:
    Monitor() : isResourceAvailable(false) {}

    void setResource(const T& newResource) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            resource = newResource;
            isResourceAvailable = true;
        }
        condition.notify_one();
    }

    T getResource() {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this]() { return isResourceAvailable; });

        isResourceAvailable = false;
        return resource;
    }
};

class MessageProducer {
private:
    Monitor<std::string>& monitor;
    int messageNum;

public:
    MessageProducer(Monitor<std::string>& monitor) : monitor(monitor), messageNum(0) {}

    void produceMessage() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::string message = "Сообщение " + std::to_string(messageNum);
            std::cout << "Отправлено: " << message << std::endl;

            monitor.setResource(message);
            messageNum++;
        }
    }
};

class MessageConsumer {
private:
    Monitor<std::string>& monitor;

public:
    MessageConsumer(Monitor<std::string>& monitor) : monitor(monitor) {}

    void consumeMessage() {
        while (true) {
            std::string message = monitor.getResource();
            std::cout << "Обработано: " << message << std::endl;
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    Monitor<std::string> messageMonitor;
    MessageProducer producer(messageMonitor);
    MessageConsumer consumer(messageMonitor);

    std::thread producerThread(&MessageProducer::produceMessage, &producer);
    std::thread consumerThread(&MessageConsumer::consumeMessage, &consumer);

    producerThread.join();
    consumerThread.join();

    return 0;
}