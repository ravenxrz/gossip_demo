#pragma once

#include "error.h"

#include <cstdint>

class TaskWorker;

class BaseTask {
public:
  BaseTask() { SetTaskState(TaskInit); }

  virtual ~BaseTask() = default;

  virtual void Run() = 0;

  void SetTaskState(uint32_t state) { task_state_ = state; }

  uint32_t GetTaskState() const { return task_state_; }

  void SetTaskStatus(uint32_t status) { task_status_ = status; }

  uint32_t GetTaskTatus() const { return task_status_; }

  void SetTaskType(uint32_t task_type) { task_type = task_type; }

  void SetWorker(TaskWorker *worker) { worker_ = worker; }

  void SetTimeTicket(int64_t ts) { time_ticket_ = ts; }

  int64_t GetTimeTicket() const { return time_ticket_; }

protected:
  enum TaskState { TaskInit, TaskFin };

  uint32_t task_state_{TaskInit};
  uint32_t task_status_{OK};
  uint32_t task_type_;
  uint64_t time_ticket_;
  TaskWorker *worker_;
};
