/*
Copyright 2013 Henrik Mühe

This file is part of Fekaton.

Fekaton is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fekaton is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with Fekaton.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Timer.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


struct System
{   
    /// Execute callback for all files at all levels of a directory
    static void for_files(const std::string& path,std::function<void (const std::string&)> callback)  {
        char *paths[] = {const_cast<char*>(path.c_str()), 0};
        FTS *tree = fts_open(paths, FTS_NOCHDIR, 0);
        if (!tree) {
            perror("fts_open");
            throw;
        }

        FTSENT *node;
        while ((node = fts_read(tree))) {
            if (node->fts_level > 0 && node->fts_name[0] == '.')
                fts_set(tree, node, FTS_SKIP);
            else if (node->fts_info & FTS_F) {
                callback(node->fts_path);
            }
        }
        if (errno) {
            perror("fts_read");
            throw;
        }

        if (fts_close(tree)) {
            perror("fts_close");
            throw;
        }
    }
    /// Retrieve 
    static int64_t getMem() {
        int64_t pageCount=sysconf(_SC_PHYS_PAGES);
        int64_t availableCount=sysconf(_SC_AVPHYS_PAGES);
        int64_t pageSize=sysconf(_SC_PAGESIZE);

        return (pageCount-availableCount)*pageSize;
    }

    static int64_t profile(const std::string& name,std::function<void()> body) {
        std::string filename;
        if (name.find(".data") == std::string::npos)
            filename = name + ".data";
        else
            filename = name;

        // Launch profiler
        pid_t pid;
        std::stringstream s;
        s << getpid();
        pid = fork();
        if (pid == 0) {
            auto fd=open("/dev/null",O_RDWR);
            dup2(fd,1);
            dup2(fd,2);
            exit(execl("/usr/bin/perf","perf","record","-o",filename.c_str(),"-p",s.str().c_str(),nullptr));
        }

        // Run body
        Timer t;
        auto mem=getMem();
        body();
        auto micros = t.getMicro();
        mem = getMem() - mem;
        std::cout << std::setw(30) << std::left << (name + ": ");
        std::cout << std::setw(10) << std::right << t.getMicro()/1000 << "ms";
        std::cout << std::setw(10) << std::right << mem/(1024*1024) << "mb" << std::endl;

        // Kill profiler  
        kill(pid,SIGTERM);
        waitpid(pid,nullptr,0);

        return micros;
    }

    static void profile(std::function<void()> body) {
        profile("perf.data",body);
    }
};

