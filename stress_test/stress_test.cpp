
#include <iostream>
#include <chrono> // for time measurement
#include <cstdlib>
#include <pthread.h> // multi-threading
#include <time.h> 
#include "../skip_list.h"

#define NUM_THREADS 1 // number of threads
#define TEST_COUNT 100000 // number of test data
SkipList<int, std::string> skipList(18); // create a skip list with max level 18
 
// insert element into skip list for thread function
void *insertElement(void* threadid) {
    long tid; // thread id
    tid = (long)threadid; // viod* to long
    std::cout << tid << std::endl;  // print thread id
    int tmp = TEST_COUNT/NUM_THREADS; // number of test data per thread
	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
		skipList.insert_element(rand() % TEST_COUNT, "a"); // insert element 'a' with random key
	}
    pthread_exit(NULL); // exit thread
}


// get element from skip list for thread function
void *getElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    int tmp = TEST_COUNT/NUM_THREADS; 
	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
		skipList.search_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}

int main() {
    srand (time(NULL));  // random seed
    {
        pthread_t threads[NUM_THREADS]; // thread array
        int rc; //receive pthread_create return value
        int i; // loop variable

        auto start = std::chrono::high_resolution_clock::now(); // start time

        // create insert variables for threads
        for( i = 0; i < NUM_THREADS; i++ ) {
            std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, insertElement, (void *)i); // create thread

            if (rc) {
                std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1); // exit if error
            }
        }

        void *ret; // return value of pthread_join
        // wait for threads to finish
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                perror("pthread_create() error"); 
                exit(3);
            }
        }
        auto finish = std::chrono::high_resolution_clock::now(); // end time
        std::chrono::duration<double> elapsed = finish - start; // calculate elapsed time
        std::cout << "insert elapsed:" << elapsed.count() << std::endl; 
    }

    // similar to the above code, but for search procedure
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        int i;

        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, getElement, (void *)i);

            if (rc) {
                std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                perror("pthread_create() error"); 
                exit(3);
            }
        }
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "search elapsed:" << elapsed.count() << std::endl;
    }
    

	pthread_exit(NULL);
    return 0;

}