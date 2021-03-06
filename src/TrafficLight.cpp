#include "TrafficLight.h"
#include <iostream>
#include <random>
#include <future>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  std::unique_lock<std::mutex> uLock(_mutex);
  _cond.wait(uLock, [this] { return !_queue.empty(); });
  // to wait for and receive new messages and pull them from the queue using
  // move semantics.
  T msg = std::move(_queue.front());
  _queue.pop_front();
  // The received object should then be returned by the receive function.
  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // FP.4a : The method send should use the mechanisms
  std::lock_guard<std::mutex> uLock(_mutex);
  // as well as _condition.notify_one() to add a new message to the queue and
  // afterwards send a notification.
  MessageQueue<T>::_queue.push_back(std::move(msg));
  _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly. calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns
  while (true) {
    TrafficLightPhase msg = _messages.receive();
    if (msg == TrafficLightPhase::green)
      return;
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be started
  // in a thread when the public method „simulate“ is called. To do this, use
  // the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics. The cycle duration should be a random value between 4 and 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.
  // int64_t toggleTime{5};

  // Random limits declaration
  const int maximum_number = 6;
  const int minimum_number = 4;

  // Calculation of random time treshold for traffic lights
  int64_t toggleTime =
      (std::rand() % (maximum_number + 1 - minimum_number)) + minimum_number;
  // Set previous time for duration calculation
  std::chrono::high_resolution_clock::time_point previousTime =
      std::chrono::high_resolution_clock::now();

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::chrono::high_resolution_clock::time_point currentTime =
        std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                        currentTime - previousTime)
                        .count();

    if (duration >= toggleTime) {
      if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
      } else {
        _currentPhase = TrafficLightPhase::red;
      }
      auto future = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, &_messages, std::move(_currentPhase));
			future.wait();
      //_messages.send(std::move(_currentPhase));
      previousTime = currentTime;
    }
  }
}
