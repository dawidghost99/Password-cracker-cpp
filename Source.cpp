#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <thread>
#include <chrono>
#include <array>
#include "windows.h"
#include <stdio.h>
#include <vector>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <cstring>

#include "md5.h"

std::string hash = "";
std::atomic<bool> found (false);
/*I have an atomic boolean that stops that informs the threads that the string has been found and that they should finish what they were doing and finish. */

using std::chrono::duration_cast;
using std::chrono::milliseconds;
typedef std::chrono::steady_clock the_clock;

static const int num_threads = std::thread::hardware_concurrency();

std::string hasherBF(std::string);
std::string hasher(std::string);
void Generate(unsigned int, std::string);
void Crack(static unsigned int);


std::mutex display_mutex;



std::mutex mtx;
std::condition_variable cv;

bool ready = false;
bool processed = false;



const char AlphabetUpper[26] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U',
	'V', 'W', 'X', 'Y', 'Z'
};

const char AlphabetLower[26] =
{
	'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u',
	'v', 'w', 'x', 'y', 'z'
};

const char Numbers[10] =
{
	'1', '2', '3', '4', '5', '6', '7',
	'8', '9', '0'
};

const char specialChars[10] =
{
	'_', '.', '-', '!', '@', '*',
	'$', '?', '&', '%'
};

//Adapted and changed code from: http://www.askyb.com/cpp/openssl-md5-hashing-example-in-cpp/
std::string hasher(std::string word) {
	std::stringstream ss;
	unsigned char digest[MD5_DIGEST_LENGTH];
	int size = word.length();
	char wordar[100];
	strcpy(wordar, word.c_str());



	//MD5((unsigned char*)&wordar, strlen(wordar), (unsigned char*)&digest);
	md5((unsigned char*)&wordar, strlen(wordar), (unsigned char*)&digest);


	for (int i = 0; i < 16; i++)
		sprintf(&wordar[i * 2], "%02x", (unsigned int)digest[i]);

	std::string hashedword;
	ss << wordar;
	ss >> hashedword;


	return(hashedword);

}

// dictionary attack on a md5 hash list.
void md5_DA() {
	std::string line;
	std::stringstream ss;
	std::vector<std::string> words;
	std::ifstream myfile;
	
	myfile.open("TextFile1.txt", std::ios::in);

	if (!myfile) {
		std::cout << "Unable to open file TextFile1.txt";
		system("pause");
		exit(1);
	}

	if (myfile.is_open()) {
		while (myfile.good()) {
			getline(myfile, line);
			words.push_back(line);
		}
		myfile.close();
	}

	

	int x = 0;


	while (x < words.size() && found == false) {
	
		std::string word = words[x];
		std::string hashedword = hasher(word);

		if (hashedword == hash){
		
			found = true;
			display_mutex.lock();
			std::cout << "found! " << std::endl;
			std::cout << "the word is: " << words[x] << std::endl;
			display_mutex.unlock();
			
			
			//system("pause");
			
		}
		x += 1;
		//std::unique_lock<std::mutex> lck(mtx);
		//cv.notify_all();
	}

	return;
	

}
//end of md_5DA

// adapted and changed from http://www.cplusplus.com/forum/lounge/151573/
void Crack(static unsigned int stringlength)
{
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, [] {return ready; });
	while (!found)
	{
		// Keep growing till it gets it right
		
		Generate(stringlength, "");
		stringlength++;
	}

	processed = true;
	cv.notify_one();
	return;
}

void Generate(unsigned int length, std::string s)
{
	if (!found) {
		
		//std::cout << s << std::endl;
		if (length == 0)
		{
			std::string hashed_word = hasher(s);
			if (hash == hashed_word) {
				found = true;
				display_mutex.lock();
				std::cout << "found!" << std::endl << "password: " << s << std::endl;
				display_mutex.unlock();
				/*I use this mutex to prevent text over lapping eachother. Becuase this function is used by multiple threads I want the text to be displayed in an oderly mannor hence why the mutexes are used*/
				return;
			}
			
			return;
		}

		for (unsigned int i = 0; i < 26; i++)
		{
			std::string appended = s + AlphabetLower[i];
			Generate(length - 1, appended);

		}


		for (unsigned int i = 0; i < 10; i++)
		{
			std::string appended = s + Numbers[i];
			Generate(length - 1, appended);

		}


		for (unsigned int i = 0; i < 26; i++)
		{
			std::string appended = s + AlphabetUpper[i];
			Generate(length - 1, appended);

		}


		for (unsigned int i = 0; i < 26; i++)
		{
			std::string appended = s + specialChars[i];
			Generate(length - 1, appended);

		}
		
	}
	return;
	
}

void myassert(bool val)
{
	if (!val)
	{
		int *a = 0;
		*a = 42;
	}
}

//end of bruteforce
void main()
{

	int threads = std::thread::hardware_concurrency();
	std::cout << "you have " << threads << " threads running" << std::endl;
	std::cout << "Hash you want to search for (space sensetive): ";
	std::cin >> hash;
	std::cout << "running..." << std::endl;

	


	std::vector <std::thread> threadarr;
	the_clock::time_point start = the_clock::now();
	threadarr.push_back(std::thread(md5_DA));

	for (int i = 1; i < threads; i++) {
		threadarr.push_back(std::thread(Crack,i));
	}
	
	{	std::lock_guard<std::mutex> lock(mtx);
	ready = true; }
	cv.notify_one();

	/*
	The std::lock_guard owns the mutex for the duration of a scoped block. Once the lock is out out scope, the mutex is automatically released. I lock the mutex and
	assign the value of the boolean "ready" to true. This is indicates that the data is ready to be processed. I then notify one of the threads that are waiting
	for this condition to happen. I then lock the mutex again using unique_lock and signal to the waiting threads that the mutex can be unlocked once the value
	of "processed" becomes true. 

	In the thread function, I wait until the main function sends data i.e. the value of "ready" becomes true. At the same time, I lock the mutex so no other threads
	can access the thread function until the calculations are done and the value of "processed" becomes true. Once the value becomes true, I can unlock the mutex
	and notify the next waiting thread that it can now perform the required calculations without accessing valuable data all at the same time as another thread.
	*/

	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] {return processed; });
	}

	for (int x = 0; x < threads; x++) {
		threadarr[x].join();
	}
	the_clock::time_point end = the_clock::now();
	auto time_taken = duration_cast<milliseconds>(end - start).count();
	std::cout << "The decryption took " << time_taken << " ms." << std::endl;
	system("pause");
	return;
}