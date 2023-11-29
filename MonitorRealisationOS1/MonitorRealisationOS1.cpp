#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

class MessageMonitor {
private:
    std::string message;
    bool isMessageAvailable;
    std::mutex mutex;
    std::condition_variable condition;
    int messageNum;

public:
    MessageMonitor() : isMessageAvailable(false), messageNum(0) {}

    void produceMessage() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            {
                std::lock_guard<std::mutex> lock(mutex);
                message = "Сообщение " + std::to_string(messageNum);
                std::cout << "Отправлено: " << message << std::endl;
                isMessageAvailable = true;
                messageNum++;
                condition.notify_one();
            }
        }
    }

    void consumeMessage() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this]() { return isMessageAvailable; });

            std::cout << "Обработано: " << message << std::endl;
            isMessageAvailable = false;
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    MessageMonitor messageMonitor;

    std::thread producerThread(&MessageMonitor::produceMessage, &messageMonitor);
    std::thread consumerThread(&MessageMonitor::consumeMessage, &messageMonitor);

    producerThread.join();
    consumerThread.join();

    return 0;
}