#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <regex>



class upgrade_ku5p
{
public:
    upgrade_ku5p(std::string file_name);
    int upgrade();
    ~upgrade_ku5p();

private:
    std::string ku5pbit_file_name;
};

int upgrade_ku5p::upgrade()
{
    std::string cmd = "";
    FILE* fp = NULL;
    char buf[256] = {'0'};
	std::string result = "";
    int percentage = 0;
    int index = 0;
    int total = 0;

    cmd = "echo 0 > /proc/spi-nor/select";
    system(cmd.c_str());

    cmd = "flashcp -v " + ku5pbit_file_name + " /dev/mtd0";
    std::cout << cmd << std::endl;

    if( (fp = popen(cmd.c_str(), "r")) == NULL ) {
        printf("popen error!\n");
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        result += buf;
        printf("buf:%s\n", buf);
        memset(buf, 0, sizeof(buf));
    }
    pclose(fp);

    std::regex pattern(R"(Verifying data: (\d+)k/(\d+).*(100))"); // 匹配Verifying data: 13312k/13312k (100%)中的校验进度值

    std::sregex_iterator it(result.begin(), result.end(), pattern);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        std::smatch match = *it;
        // std::cout << "Match found: " << match.str() << std::endl;
        index = std::stoi(match[1].str());
        total = std::stoi(match[2].str());
        percentage = std::stoi(match[3].str());
        //printf("index = %d, total = %d, percentage = %d\n", index, total, percentage);
    }
    printf("index = %d, total = %d, percentage = %d\n", index, total, percentage);
    if (index == total && percentage == 100) 
        return 0;
    else return -1;
}

upgrade_ku5p::upgrade_ku5p(std::string file_name)
{
    ku5pbit_file_name = file_name;
}

upgrade_ku5p::~upgrade_ku5p()
{

}


int main(int argc, char *argv[])
{
    int ret = -1;
    if(argc < 2) {
        std::cout << "please input ku5p file name !" << std::endl;
        return 0;
    }
    printf("upgrade ku5p file name:%s\n", argv[1]);
    upgrade_ku5p upgrade_ku5p_ojb(argv[1]);
    ret = upgrade_ku5p_ojb.upgrade();
    if(ret == 0) {
        printf("ku5p upgrade success!\n");
    }else {
        printf("ku5p upgrade failed!\n");
    }

    return 0;

}
