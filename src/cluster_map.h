#pragma once

#include <map>
#include <string>
#include <vector>

namespace redis {
struct Server {
  std::string ip;
  int port;

  Server();
  void Clear();
  bool IsValid() const;
  std::string ToString() const;
  bool Parse(const std::string &addr);
};
class Slot {
 private:
  int beg_;
  int end_;
  Server master_;
  std::vector<Server> slaves_;

 public:
  Slot();
  ~Slot();
  void Clear();

  bool IsValid() const;

  int Begin() const;
  int End() const;
  void Begin(int beg);
  void End(int end);

  const Server &Master() const;
  void Master(const std::string &ip, int port);
  void Master(const Server &);
  void Master(Server &&);

  int SlaveNum() const;
  const Server &Slave(int i) const;
  void AddSlave(const std::string &ip, int port);
  void AddSlave(const Server &);
  void AddSlave(Server &&);

  void Print() const;
};

class SlotMap {
 public:
  SlotMap();
  ~SlotMap();
  void Clear();
  void Freeze();

  bool Add(const Slot &slot);

  const Slot *Find(int slot) const;
  int GetSlot(const std::string &key) const;
  Server FindWrite(int slot) const;
  Server FindRead(int slot, int master_ratio = 0) const;

  void SetClusterName(const std::string &name);
  const std::string &GetClusterName() const;

  std::map<int, Slot>::const_iterator Begin() const;
  std::map<int, Slot>::const_iterator End() const;

  void Print() const;
  int InitChannel();
  int GetSlotFromServer(const Server &);

 public:
  int Load(const std::string &content);

 private:
  std::map<int, Slot> slot_map_;
  std::map<int, Slot>::const_iterator beg_;
  std::string cluster_name_;
};
}  // namespace redis
