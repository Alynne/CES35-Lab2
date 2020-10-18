#include <iostream>
#include "client.h"

int main(int argc, char* argv[]) {
    
    
    if (argc < 2) { // We expect at least one URL
        std::cerr << "Error: At least one URL expected." << std::endl;
        return 1;
    }
    for (int i = 1; i < argc; i++){
        auto url = http::url::parse(argv[i]);
        
        if (url.has_value()){
            if((url->getScheme() == "http") || (url->getScheme() == "https")){
                std::string host = std::string(url->getHost());
                std::uint16_t port = url->port;
                http_client myClient(host, port);
                std::cout << "Requesting /" << url->getPath() << " from " << url->getHost() << " " << url->port << std::endl;
                myClient.get(*url);
                fs::path resourcePath(url->getPath());
                std::string outputPath = "./";
                if (resourcePath.string().empty() || fs::is_directory(resourcePath)) {
                    outputPath += "index.html";
                } else {
                    outputPath += resourcePath.filename().string();
                }
                myClient.saveAt(outputPath);
            }
            else{
                std::cerr<< "Error: Not http."<< std::endl;
            }

        }
        else {
            std::cerr<< "Error: Unable to parse URL."<< std::endl;
        }
    }
    return 0;
}