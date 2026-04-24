#include "Recorder.hpp"

#include <iostream>

Recorder::~Recorder() {
  for (auto& kv : lines_) {
    delete kv.second;
  }
}

void Recorder::add(int line, Statement* stmt) {
  auto it = lines_.find(line);
  if (it != lines_.end()) {
    delete it->second;
  }
  lines_[line] = stmt;
}

void Recorder::remove(int line) {
  auto it = lines_.find(line);
  if (it != lines_.end()) {
    delete it->second;
    lines_.erase(it);
  }
}

const Statement* Recorder::get(int line) const noexcept {
  auto it = lines_.find(line);
  if (it == lines_.end()) return nullptr;
  return it->second;
}

bool Recorder::hasLine(int line) const noexcept {
  return lines_.find(line) != lines_.end();
}

void Recorder::clear() noexcept {
  for (auto& kv : lines_) delete kv.second;
  lines_.clear();
}

void Recorder::printLines() const {
  for (auto& kv : lines_) {
    std::cout << kv.first << " " << kv.second->text() << "\n";
  }
}

int Recorder::nextLine(int line) const noexcept {
  auto it = lines_.upper_bound(line);
  if (it == lines_.end()) return -1;
  return it->first;
}

