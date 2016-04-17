#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>
#include "common.h"
#include "led_driver.h"

using namespace std;
#define FIFO_CMD "/tmp/led_fifo_cmd"
#define FIFO_RESP "/tmp/led_fifo_resp"

LedDriver *leddrv = 0;

bool handle_command(const string &cmd, const string &arg, string &resp_arg)
{
    if (cmd == "get-led-state")
    {
        resp_arg = leddrv->get_state() ? "on" : "off";
    }
    else if (cmd == "get-led-color")
    {
        resp_arg = ColorToStr(leddrv->get_color());
    }
    else if (cmd == "get-led-rate")
    {
        std::ostringstream oss;
        oss << leddrv->get_rate();
        resp_arg = oss.str();
    }
    else if (cmd == "set-led-state")
    {
        if (arg == "on")
            leddrv->set_state(true);
        else if (arg == "off")
            leddrv->set_state(false);
        else return false;
    }
    else if (cmd == "set-led-color")
    {
        LedColor clr = StrToColor(arg);
        if (clr == LED_UNKNOWN) return false;
        leddrv->set_color(clr);
    }
    else if (cmd == "set-led-rate")
    {
        int rate = 0;
        istringstream iss(arg);
        iss >> rate;
        if (iss.fail() || !iss.eof() || !IsCorrectRate(rate)) return false;
        leddrv->set_rate(rate);
    }
    else return false;

    return true;
}

string handle_line(const string &line)
{
    vector<string> tokens;
    split(line, tokens);

    string cmd;
    string arg;

    if (tokens.size() > 0) cmd = tokens[0];
    if (tokens.size() > 1) arg = tokens[1];
    if (tokens.size() > 2) return "FAILED";

    string resp_arg;
    if (handle_command(cmd, arg, resp_arg))
    {
        if (resp_arg.empty())
            return "OK";
        else
            return "OK " + resp_arg;
    }
    else
    {
        return "FAILED";
    }
}

int setup_fifos(std::fstream &fifo_cmd, std::fstream &fifo_resp)
{
    unlink(FIFO_CMD);
    unlink(FIFO_RESP);

    int ret;

    ret = mkfifo(FIFO_CMD, 0666);
    if (ret)
    {
        std::cerr << "error mkfifo, file " << FIFO_CMD
                  << ", err code = " << ret << endl;
        return 1;
    }

    ret = mkfifo(FIFO_RESP, 0666);
    if (ret)
    {
        std::cerr << "error mkfifo, file " << FIFO_RESP
                  << ", err code = " << ret << endl;
        return 1;
    }

    fifo_cmd.open(FIFO_CMD);
    if (!fifo_cmd.is_open())
    {
        std::cerr << "error open file " << FIFO_CMD << endl;
        return 1;
    }

    fifo_resp.open(FIFO_RESP);
    if (!fifo_resp.is_open())
    {
        std::cerr << "error open file " << FIFO_RESP << endl;
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    std::fstream fifo_cmd;
    std::fstream fifo_resp;

    int ret = setup_fifos(fifo_cmd, fifo_resp);
    if (ret) return ret;

    leddrv = new LedDriver();
    leddrv->start_thread();

    while (true)
    {
        string line;
        if (!getline(fifo_cmd, line))
        {
            std::cerr << "error: cannot get line from FIFO "
                      << FIFO_CMD << endl;
            return 1;
        }
        string resp = handle_line(line);
        fifo_resp << resp << endl;
    }

    /*In real project I would add exit command to server and intercept
        SIGTERM for graceful shutdown (delete FIFOs)*/

    return 0;
}
