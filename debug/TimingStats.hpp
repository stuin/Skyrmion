#define MAX_DELTA_TIMES 200

struct TimingStats {
    unsigned int totalCount = 0;
    double totalTime = 0;
    double delta[MAX_DELTA_TIMES];
    double maxDelta = 0;

    void addDelta(double _delta) {
        totalTime += _delta;
        delta[(totalCount++) % MAX_DELTA_TIMES] = _delta;

        if(_delta > maxDelta)
            maxDelta = _delta;
    }

    int getFPS() {
        unsigned int start = totalCount % MAX_DELTA_TIMES;
        unsigned int frames = 0;
        double seconds = 0;
        while(frames < MAX_DELTA_TIMES && seconds < 1.0)
            seconds += delta[(start + frames++) % MAX_DELTA_TIMES];
        return frames;
    }

    double last() {
        return delta[totalCount % MAX_DELTA_TIMES];
    }
};