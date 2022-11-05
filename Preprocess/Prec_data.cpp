//
// Created by elssky on 2022/11/5.
//
#include"Prec_data.h"


//Preprocessing function

int get_vertex_id(int u, int &num, int *vertex_map_) {
    if (vertex_map_[u] < 0) {
        vertex_map_[u] = num++;
    }
    return vertex_map_[u];
}

void format_graph(string src) {
    printf("Create binary graph from .txt file: %s\n", src.c_str());

    // input example "./dataset/stanford.txt"
    string::size_type pos = src.rfind("/");

    // get file direcotry
    // e.g. "./dataset/"
    string file_dir = src.substr(0, pos + 1);

    // get graph_name
    // e.g. "stanford"
    string graph_name = src.substr(pos + 1, src.rfind(".") - pos - 1);

    // create out file
    // e.g. "./dataset/stanford"
    string out = file_dir + graph_name + "_Processed" + ".txt";

    ifstream fp;
    ofstream fp2;
    fp.open(src.c_str());
    fp2.open(out.c_str());
    //FILE* fp = open_file(src.c_str(), "r");
    //FILE* fp2 = open_file(out.c_str(), "w");
    string line;
    long nodes = 0, edges = 0;
    int u, v;
    int num = 0;

    while (getline(fp, line)) {
        //fp2 << line << endl;
        if (line[2] == 'F')
            break;
        if (line[2] == 'N') {
            string::size_type pos1 = line.find("Nodes: ") + 7;
            string::size_type blank1 = line.find(" ", pos1);
            string nodes_num = line.substr(pos1, blank1 - pos1);
            string::size_type pos2 = line.find("Edges: ") + 7;
            string::size_type blank2 = line.find("\r", pos2);
            string edges_num = line.substr(pos2, blank2 - pos2);
            stringstream ss1(nodes_num);
            stringstream ss2(edges_num);
            ss1 >> nodes;
            ss2 >> edges;
            cout << "Nodes_Num:" << nodes << endl;
            cout << "Edges_Num:" << edges << endl;
            fp2 << nodes << " " << edges << endl;


        }


    }
    int *vertex_map_ = new int[1000000];
    std::memset(vertex_map_, -1, sizeof(int) * 1000000);


    while (getline(fp, line)) {

        if (line[0] < '0' || line[0] > '9')
            continue;

        sscanf(line.c_str(), "%d\t%d", &u, &v);

        if (u == v) continue;


        u = get_vertex_id(u, num, vertex_map_);
        v = get_vertex_id(v, num, vertex_map_);
        cout << num << endl;

        //fprintf(fp2, "%d\t%d", u, v);
        fp2 << u << "\t" << v << endl;

    }

    int *new2old = new int[num];
    int cur = 0;
    int i = 0;
    while (cur < num) {
        if (vertex_map_[i] == -1) {
            ++i;
            continue;
        }
        new2old[vertex_map_[i]] = i;
        ++cur;
        ++i;
    }
    string mts = "./dataset/match.st";
    FILE *mch;
    fopen(mts.c_str(), "wb");
    //FILE* mch = fopen(mts.c_str(), "wb");
    if (mch) {
        fwrite(new2old, sizeof(int), num, mch);
        fclose(mch);
    }

    delete[] new2old;
    delete[] vertex_map_;



    /*read part*/
    //FILE *mch2;
    //int* new2old = new int[num];
    //err = fopen_s(&mch2, mts.c_str(), "rb");
    //fread(new2, sizeof(int), num, mch);
    //fclose(mch);

    //ifstream fp3;
    //fp3.open(out.c_str());
    //while (getline(fp3, line)) {//fgets(line, 100, fp)

    //	if (line[0] < '0' || line[0] > '9')
    //		continue;

    //	sscanf_s(line.c_str(), "%d\t%d", &u, &v);

    //	if (u == v) continue;
    //	u = new2old[u];
    //	v = new2old[v];
    //	cout << u << "\t" << v << endl;
    //}


}

int main(int argc, char **argv) {
    char *argv1;
    argv1 = argv[1];
    format_graph(argv1);
}



