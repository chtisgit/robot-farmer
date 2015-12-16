#include "threadpool.h"


Worker::Worker(int id, std::thread&& thr) : id(id), thr(move(thr))
{
}

