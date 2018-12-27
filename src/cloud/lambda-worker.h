#ifndef PBRT_CLOUD_LAMBDA_WORKER_H
#define PBRT_CLOUD_LAMBDA_WORKER_H

#include <deque>
#include <iostream>
#include <string>

#include "cloud/bvh.h"
#include "cloud/lambda.h"
#include "cloud/raystate.h"
#include "core/camera.h"
#include "core/geometry.h"
#include "core/light.h"
#include "core/sampler.h"
#include "core/transform.h"
#include "execution/loop.h"
#include "execution/meow/message.h"
#include "net/address.h"
#include "storage/backend.h"
#include "util/temp_dir.h"

namespace pbrt {

class LambdaWorker {
  public:
    LambdaWorker(const std::string& coordinatorIP,
                 const uint16_t coordinatorPort,
                 const std::string& storageBackendUri);

    void run();

  private:
    struct Worker {
        enum class State { Connecting, Connected };

        WorkerId id;
        Address address;
        State state{State::Connecting};
        int32_t seed{0};

        std::set<TreeletId> treelets{};

        Worker(const WorkerId id, Address&& addr)
            : id(id), address(std::move(addr)) {}
    };

    bool processMessage(const meow::Message& message);
    void initializeScene();

    void loadCamera();
    void loadSampler();
    void loadLights();
    void loadFakeScene();

    Poller::Action::Result::Type handleRayQueue();
    Poller::Action::Result::Type handleOutQueue();
    Poller::Action::Result::Type handleFinishedQueue();
    Poller::Action::Result::Type handlePeers();
    Poller::Action::Result::Type handleMessages();
    Poller::Action::Result::Type handleNeededTreelets();

    void generateRays(const Bounds2i& cropWindow);

    Address coordinatorAddr;
    ExecutionLoop loop{};
    UniqueDirectory workingDirectory;
    std::unique_ptr<StorageBackend> storageBackend;
    std::shared_ptr<TCPConnection> coordinatorConnection;
    std::shared_ptr<UDPConnection> udpConnection;
    meow::MessageParser messageParser{};
    Optional<WorkerId> workerId;
    std::map<WorkerId, Worker> peers;
    int32_t mySeed;
    bool peerRequested{false};

    /* Scene Data */
    bool initialized{false};
    std::vector<std::unique_ptr<Transform>> transformCache{};
    std::shared_ptr<Camera> camera{};
    std::unique_ptr<FilmTile> filmTile{};
    std::shared_ptr<Sampler> sampler{};
    std::unique_ptr<Scene> fakeScene{};
    std::vector<std::shared_ptr<Light>> lights{};
    std::shared_ptr<CloudBVH> treelet{};
    MemoryArena arena;

    /* Rays */
    std::deque<RayState> rayQueue{};
    std::deque<RayState> finishedQueue{};
    std::map<TreeletId, std::deque<RayState>> pendingQueue{};
    std::map<TreeletId, std::deque<RayState>> outQueue{};
    size_t outQueueSize{0};

    std::map<TreeletId, WorkerId> treeletToWorker{};
    std::set<TreeletId> neededTreelets;
    std::set<TreeletId> requestedTreelets;

    /* Always-on FD */
    FileDescriptor dummyFD{STDOUT_FILENO};
};

}  // namespace pbrt

#endif /* PBRT_CLOUD_LAMBDA_WORKER_H */
