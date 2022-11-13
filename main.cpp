#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>

using namespace std;

struct HeapNode {
    string line;
    int index;

    HeapNode(string a, int b): line(a), index(b) {}
    
    bool operator<(const HeapNode& rhs) const {
        return (line.compare(rhs.line) > 0);
    }
};


/**
 * Split input file into sorted chunks.
 * @param input_name the input file name
 * @param mem_limit the input memory limit
 * @return the number of chunks
 */
int createInitialChunks(string input_name, long mem_limit) {
    ifstream input; 
    input.open(input_name.c_str());

    if (!input.good()) {
        cerr << "Error: " << input_name << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    streampos input_size; 
    input.seekg(0, input.end);
    input_size = input.tellg();
    input.seekg(0, input.beg);
    cout << "The size of the input file is " << input_size << " bytes" << endl;

    int chunk_count = 0;
    long mem_so_far = 0;

    ofstream output;
    vector<string> data;

    while (!input.eof()) {
        string line;
        getline(input, line);
        if (mem_so_far + line.size() < mem_limit) {
            mem_so_far += line.size() + 1; // + 1 for newline symbol
            data.push_back(line);
        } else {
            sort(data.begin(), data.end());
            chunk_count++;
            stringstream ss;
            ss << "chunk_" << chunk_count << ".txt";
            cout << "Writing " << ss.str() << endl;
            output.open(ss.str());

            // write lines from vector to file
            // add newline between lines
            int data_size = data.size();
            for (int i = 0; i < data_size-1; i++) {
                output << data[i];
                output << endl;
            }
            output << data[data_size-1];
            output.close();

            // reset data and memory for new chunk
            data.clear();
            mem_so_far = line.size();
            data.push_back(line);
        }
    }   

    // handle the remaining lines
    if (data.size() > 0) {
        sort(data.begin(), data.end());
        chunk_count++;
        stringstream ss;
        ss << "chunk_" << chunk_count << ".txt";
        cout << "Writing " << ss.str() << endl;
        output.open(ss.str());

        int data_size = data.size();
        for (int i = 0; i < data_size-1; i++) {
            output << data[i];
            output << endl;
        }
        output << data[data_size-1];

        input.close();
        output.close();
    }

    cout << "Done splitting file to chunks and sorting each chunks!" << endl;
    return chunk_count;
}

void merge(int start, int end, int destination) {

    int chunk_count = end - start + 1;

    ifstream input[chunk_count];
    for (int i = 0; i < chunk_count; i++) {
        stringstream ss;
        ss << "chunk_" << start + i << ".txt";
        input[i].open(ss.str());
    }
    // cout << "Hello";
    ofstream output;
    stringstream ss;
    ss << "chunk_" << destination << ".txt";
    output.open(ss.str());

    priority_queue<HeapNode, vector<HeapNode> > pq;
    for (int i = 0; i < chunk_count; i++) {
        string line;
        if (!input[i].eof()) {
            getline(input[i], line);
            pq.push(HeapNode(line, i));
        }
    }

    cout << endl << "Merging from chunk_" << start << " to chunk_" << end << " into chunk_" << destination << " file" << endl;
    while (!pq.empty()) {
        string line = pq.top().line; 
        int index = pq.top().index;
        pq.pop();
        output << line << endl;
        
        if (!input[index].eof()) {
            getline(input[index], line);
            pq.push(HeapNode(line, index));
        }
    }
    cout << "Done merging!" << endl;

    // close input and output file stream
    for (int i = 0; i < chunk_count; i++) {
        input[i].close();
    }
    output.close();
}

void mergeFiles(int chunk_count, string output_name) {
	cout << "Merging " << chunk_count << " files into " << output_name << endl;

    int distance = 100;
    int start = 1;
    int end = chunk_count;
    while (start < end) {
        int destination = end;
        int time = (end - start + 1) / distance + 1;
        if ((end - start + 1) / time < distance) {
            distance = (end - start + 1) / time + 1;
        }
        while (start <= end) {
            int mid = min(start + distance, end);
            destination++;
            merge(start, mid, destination);
            start = mid + 1;
        }
        end = destination;
    }

    stringstream ss;
    ss << "chunk_" << start << ".txt";
    rename(ss.str().c_str(), output_name.c_str());

    cout << "Removing chucks files!" << endl;
    for (int i = 1; i < end; i++) {
        stringstream ss;
        ss << "chunk_" << i << ".txt";
        cout << "Removing " << ss.str() << endl;
        remove(ss.str().c_str());
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: ./main input_file output_file mem_size" << endl;
        exit(EXIT_FAILURE);
    }

    string input_name = argv[1];
    string output_name = argv[2];
    long mem_limit = strtol(argv[3], nullptr, 0); // bytes 
    int chunk_count = createInitialChunks(input_name, mem_limit);
    mergeFiles(chunk_count, output_name);
    
    return 0;
}
