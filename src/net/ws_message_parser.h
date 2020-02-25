/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef PBRT_NET_WS_MESSAGE_PARSER_H
#define PBRT_NET_WS_MESSAGE_PARSER_H

#include <list>
#include <queue>
#include <string>

#include "ws_frame.h"
#include "ws_message.h"

class WSMessageParser {
  private:
    std::string raw_buffer_{};
    std::list<WSFrame> frame_buffer_{};
    std::queue<WSMessage> complete_messages_{};

  public:
    void parse(const std::string& buf);

    bool empty() const { return complete_messages_.empty(); }

    const WSMessage& front() const { return complete_messages_.front(); }
    WSMessage& front() { return complete_messages_.front(); }

    void pop() { complete_messages_.pop(); }
};

#endif /* PBRT_NET_WS_MESSAGE_PARSER_H */
