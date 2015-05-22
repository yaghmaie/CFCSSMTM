#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <time.h>

#define ARRSIZE 64 // Size of random array
#define THREADCNT 16 // Thread Count *** MUST BE 2 EXPONENT ***

using namespace std;

void  * mergesort( void * arg );
void  * merge( void * arg );

long int * array = new long int[ARRSIZE]; // random array

struct mergesortArg {
    // data to send to mergesort
    long int low;
    long int high;

    pthread_t supervisor;
    pthread_t sub;
};

struct mergeArg {
    // data to send to merge( i > 0 ) mergesortData[i].low++;
    long int low;
    long int mid;
    long int high;

    bool threaded;
    pthread_t *t1;
    pthread_t *t2;

    pthread_t supervisor;
    pthread_t sub;
};

int main()
{
//---------------------------------------------------------- Block  -------------------------------------------------------
    pthread_t *t = new pthread_t[THREADCNT];
    pthread_t *m = new pthread_t[THREADCNT/2];

    struct mergesortArg *mergesortData = new mergesortArg[THREADCNT];
    struct mergeArg *mergeData = new mergeArg[THREADCNT/2];

    register long int i;

    srand( time(NULL) );
    i = 0;
//---------------------------------------------------------- Block  -------------------------------------------------------
    for( ; i < ARRSIZE; i++ ) { // fill random array
        array[i] = rand() % ARRSIZE;
        cout<<array[i]<<" ";
    }
//---------------------------------------------------------- Block  -------------------------------------------------------
    cout<<endl;
    i = 0;
//---------------------------------------------------------- Block  -------------------------------------------------------
    for( ; i < THREADCNT; i++ ){
        mergesortData[i].low = i * ( ARRSIZE / THREADCNT );
        mergesortData[i].high = (i+1) * (ARRSIZE / THREADCNT) - 1;

        mergesortData[i].supervisor = t[( i + ( THREADCNT - 1 ) ) % THREADCNT ];
        mergesortData[i].sub = t[ ( i + 1 ) % THREADCNT ];
        cout<<"Sort data of :"<<i<<" low & high "<<mergesortData[i].low<<" "<<mergesortData[i].high<<endl;
    }
//---------------------------------------------------------- Block  -------------------------------------------------------

//---------------------------------------------------------- Block  -------------------------------------------------------
    i = 0;
    for( ; i < THREADCNT; i++ ) {
        pthread_create( &t[i], NULL, mergesort, (void *)&mergesortData[i] );
    }
//---------------------------------------------------------- Block  -------------------------------------------------------
    // Create mergesort threads for each quarter of random array

    //pthread_create( &t[1], NULL, mergesort, (void *)&mergesortData[1] );
    //pthread_create( &t[2], NULL, mergesort, (void *)&mergesortData[2] );
    //pthread_create( &t[3], NULL, mergesort, (void *)&mergesortData[3] );

    // Create merge threads
    int dic = ( THREADCNT / 2 );
    while ( dic > 1 ) {
        cout<<"Dic: "<<dic<<endl;
        i = 0;
        for( ; i < dic; i++ ) {
            mergeData[i].low = i * ( ARRSIZE / dic );
            mergeData[i].high = (i+1) * ( ARRSIZE / dic ) - 1;
            mergeData[i].mid =  (mergeData[i].low + mergeData[i].high) / 2;
            cout<<"Merge "<<i<<" low: "<<mergeData[i].low<<" mid: "<<mergeData[i].mid<<" high: "<<mergeData[i].high<<endl;
            mergeData[i].threaded = true;
            mergeData[i].t1 = &t[i*2];
            mergeData[i].t2 = &t[i*2+1];

            mergeData[i].supervisor = t[(i+1) % 2];
            mergeData[i].sub = t[(i)];
        }

        i = 0;
        for( ; i < dic; i++ ) {
            pthread_create( &m[i], NULL, merge, (void *)&mergeData[i]);
        }
        cout<<"Threads created\n";
        //pthread_create( &m[0], NULL, merge, (void *)&mergeData[0]);
        //pthread_create( &m[1], NULL, merge, (void *)&mergeData[1]);

        // Wait for merges to complete
        i = 0;
        for( ; i < dic; i++ ) {
            pthread_join( m[i],NULL) ;
        }
        cout<<"Session completed\n";
        dic /= 2;
    }
    //pthread_join( m[0],NULL) ;
    //pthread_join( m[1],NULL) ;
    //cout<<"Final merge"<<endl;
    cout<<"Out of while\n";
    // Do the final merge
    struct mergeArg lastmerge;
    lastmerge.low = 0; lastmerge.high = ARRSIZE - 1;
    lastmerge.mid = ARRSIZE / 2 - 1;
    lastmerge.threaded = false;

    merge( (void *) &lastmerge );

    cout<<"\nSorted :";

    i = 0;
    bool err = false;
//---------------------------------------------------------- Block  -------------------------------------------------------
    for( ; i < ARRSIZE; i++ ) {
        if( array[i] > array[i+1] && i != ARRSIZE - 1 )
            err = true;
        cout<<array[i]<<" ";
    }
//---------------------------------------------------------- Block  -------------------------------------------------------
    cout<<endl<<(err?"Error Occured\n":"Well Sorted\n");
    return 0;
}

void * mergesort( void * arg )
{
//---------------------------------------------------------- Block 01 -------------------------------------------------------
    struct mergesortArg *data;
    data = ( struct mergesortArg * ) arg;
    if( data->low < data->high ) {
//---------------------------------------------------------- Block 02 -------------------------------------------------------
        long int mid = data->low + ( data->high - data->low ) / 2;
        struct mergesortArg child1; child1.low = data->low; child1.high = mid;
        struct mergesortArg child2; child2.low = mid + 1; child2.high = data->high;

        mergesort( ( void * ) &child1 );
        mergesort( ( void * ) &child2 );

        struct mergeArg child3;
        child3.low = data->low; child3.mid = mid; child3.high = data->high; child3.threaded = false;

        merge( (void *) &child3 );
//---------------------------------------------------------- Block 03 -------------------------------------------------------
    }

    return 0;
}

void * merge( void * arg )
{
//---------------------------------------------------------- Block 11 -------------------------------------------------------
    struct mergeArg *data;
    data = ( struct mergeArg * ) arg;

    if( data->threaded ) {
//---------------------------------------------------------- Block 12 -------------------------------------------------------
        pthread_join( *data->t1,NULL) ;
        pthread_join( *data->t2,NULL) ;
        //cout<<"Merge started"<<endl;
    }
//---------------------------------------------------------- Block 13 -------------------------------------------------------
    long int h,i,j,k;
    long int * b = new long int [ARRSIZE];
    h=data->low;
    i=data->low;
    j=data->mid+1;
//---------------------------------------------------------- Block 14 -------------------------------------------------------
    while( (h <= data->mid) && (j <= data->high) ) {
        if(array[h] <= array[j]) {
//---------------------------------------------------------- Block 15 -------------------------------------------------------
            b[i]= array[h];
            h++;
//---------------------------------------------------------- Block 16 -------------------------------------------------------
        } else {
            b[i]= array[j];
            j++;
//---------------------------------------------------------- Block 17 -------------------------------------------------------
        }
        i++;
//---------------------------------------------------------- Block 18 -------------------------------------------------------
    }
    if( h>data->mid ) {
//---------------------------------------------------------- Block 19 -------------------------------------------------------
        k = j;
//---------------------------------------------------------- Block 100 -------------------------------------------------------
        for( ; k <= data->high ;k++ ) {
            b[i]=array[k];
            i++;
        }
//---------------------------------------------------------- Block 111 -------------------------------------------------------
    }
    else {
        k = h;
//---------------------------------------------------------- Block 112 -------------------------------------------------------
        for( ; k<=data->mid ;k++ ) {
            b[i]=array[k];
            i++;
        }
//---------------------------------------------------------- Block 113 -------------------------------------------------------
    }
    k = data->low;
//---------------------------------------------------------- Block 114 -------------------------------------------------------
    for( ; k<=data->high ;k++ ) array[k]=b[k];
    delete b;
//---------------------------------------------------------- Block 115 -------------------------------------------------------
    return 0;
}
