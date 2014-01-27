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

#include "SuffixArray.hpp"
#include "System.hpp"
#include <utility>


class Index {
    struct File {
        std::string filename;
        std::vector<int64_t> linebreaks;

        /// Build a file record
        File(const std::string& filename,const char* start,const char* end) : filename(filename) {
            // Find all newlines
            for (const char* ptr=start ; ptr!=end; ++ptr)
                if (*ptr == '\n')
                    linebreaks.push_back(std::distance(start,ptr));
        }

        /// Retrieve the line for the offset inside the vector
        int64_t getLine(int64_t pos) {
            auto iter=std::lower_bound(linebreaks.begin(),linebreaks.end(),pos);
            return std::distance(linebreaks.begin(),iter)+1;
        }
    };

    std::vector<int64_t> fileSkips;
    std::vector<File> files;
    bool indexed=false;
    SuffixArray sa;

public:
    void index(const std::string& path) {
	    std::vector<char> data;

        // Find all files
        System::for_files(path, [&](const std::string& file) {
        	// Read file into vector
	        ifstream f(file);
	        int64_t oldSize=data.size();
	        fileSkips.push_back(data.size());
	        data.insert(data.end(),std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>());
	        const char* start = data.data()+oldSize;
	        const char* end = data.data()+data.size();

	        // Create file record
	        std::string filename=file;
	        if (filename.length() > path.length())
	        	filename=file.substr(path.length()+1);
            files.emplace_back(filename, start, end);
        });
        fileSkips.push_back(data.size());

        // Build array
        sa.buildZeroCopy(std::move(data));

        indexed=true;
    }

    std::vector<std::string> find(const std::string& needle) {
    	std::vector<std::string> res;

        // Find all matches using the suffix array
        auto ms = sa.find(needle);
        std::unordered_set<int64_t> usedLines;
        for (auto& m : ms) {
        	// Find file
            auto iter=std::upper_bound(fileSkips.begin(),fileSkips.end(),m);
            auto fileNo=(std::distance(fileSkips.begin(),iter));
            auto& file=files[fileNo-1];

            // Fine line
            auto line=file.getLine(m-fileSkips[fileNo-1]);

            std::stringstream r; 
            if (!usedLines.count((fileNo<<32)|line)) {
                r << file.filename << ":" << line;
                usedLines.insert((fileNo<<32)|line);
            } else { continue; }
            res.push_back(r.str());
        }
        return res;
    }

    bool isIndexed() const {
        return indexed;
    }
};
