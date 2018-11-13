#include <iostream>
#include <dragonet.h>

dragonet::Dragonet dragonet_;

void callback(const int *data)
{
    std::cout << *data << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        std::cout << "No channel specified." << std::endl;
        std::cout << "Usage: " << argv[0] << " [channel]" << std::endl;
        return 0;
    }
    dragonet_.Init();
    dragonet_.Subscribe(std::string(argv[1]), callback);
    dragonet_.Spin();
}
