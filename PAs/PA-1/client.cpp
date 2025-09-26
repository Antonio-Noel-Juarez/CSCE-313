/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name:
	UIN:
	Date:
*/
#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;

void request_file_data(FIFORequestChannel* chan, char* buf, filemsg fm, string filename) {
	int len = sizeof(filemsg) + (filename.size() + 1);
	char* buf2 = new char[len];

	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), filename.c_str());
	chan->cwrite(buf2, len);

	std::memset(buf, 0, MAX_MESSAGE);              // fill with 0s
	chan->cread(buf, MAX_MESSAGE);
	delete[] buf2;
}

int main (int argc, char *argv[]) {
	// Create server child process
	pid_t pid = fork();
	if (pid == -1) {
		cerr << "fork failed\n";
		return 1;
    }

	// Child process; open up the server as child
	if (pid == 0) {
		char *argv[2];
  		argv[0] = (char *) "./server";
  		argv[1] = nullptr;
		execvp(argv[0], argv);

		// If execvp returns, something went wrong
		cerr << "Exec failed\n";
		return 1;
	}

	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;

	bool t_flag = false;
	bool e_flag = false;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				t_flag = true;
				break;
			case 'e':
				e = atoi (optarg);
				e_flag = true;
				break;
			case 'f':
				filename = optarg;
				break;
		}
	}

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	
	// example data point request
    char buf[MAX_MESSAGE]; // 256

	if (!filename.empty()) {
		// sending a non-sense message, you need to change this
		filemsg fm(0, 0);

		int len = sizeof(filemsg) + (filename.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), filename.c_str());

		chan.cwrite(buf2, len);  // I want the file length;
		delete[] buf2;

		long size;
		chan.cread(&size, sizeof(double));

		std::ofstream outFile("received/" + filename, std::ios::binary);  // opens in write mode by default
		fm.length = MAX_MESSAGE - 1;
		for (int i = 0; i < size; i += fm.length) {
			if (size - i < MAX_MESSAGE - 1) {
				fm.length = size - i;
			}
			fm.offset = i;
			request_file_data(&chan, buf, fm, filename);
			outFile.write(buf, fm.length);
		}

		outFile.close();

	} else if (t_flag || e_flag) {
    	datamsg x(p, t, e);
		
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	} else {
		std::ofstream outFile("received/x1.csv");  // opens in write mode by default
		if (!outFile) {
			std::cerr << "Error opening file\n";
			return 1;
		}
		for (t = 0; t < 0.004 * 1000; t += 0.004) {
			// Request e1
			datamsg x(p, t, 1);
			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double e1;
			chan.cread(&e1, sizeof(double)); //answer

			// Request e2
			datamsg y(p, t, 2);
			memcpy(buf, &y, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double e2;
			chan.cread(&e2, sizeof(double)); //answer
			
			outFile << t << "," << e1 << "," << e2 << "\n";
		}
		outFile.close();
	}
	
    
	
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
}
