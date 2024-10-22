#include <udpflow/CommandHandler.h>
#include <udpflow/CommandProcessor.h>
#include <udpflow/Consts.h>
#include <udpflow/Receiver.h>
#include <udpflow/Sender.h>
#include <udpflow/Server.h>
#include <udpflow/Stat.h>
#include <udpflow/Utils.h>

#include <iostream>
#include <string>
#include <thread>
// #include <stdio.h>
// #include <sys/select.h>
// #include <termios.h>
// //#include <stropts.h>

void output(const std::string & str)
{
    std::cout << str << std::endl;
}

int main(int argc, char *argv[])
{
    // const std::string server_ip = argc > 1 ? argv[1] : "127.0.0.1";

    const auto port = udpflow::constants::default_receive_port;
    const auto stat = std::make_shared<udpflow::Stat>();
    const auto command_handler = std::make_shared<udpflow::CommandHandler>(stat);
    const auto command_processor = std::make_shared<udpflow::CommandProcessor>(command_handler);
    const auto server = std::make_unique<udpflow::Server>(command_processor, stat, port);
    udpflow::OutputIPv4Addresses();
    std::cout << "Listen UDP port " << port << std::endl;
    bool stop_flag = false;
    for (;!stop_flag;)
    {
        std::this_thread::sleep_for(std::chrono::seconds{1});
        std::cout << stat->GetStat() << std::endl;
        command_handler->onTick();
    }

    return 0;
}


