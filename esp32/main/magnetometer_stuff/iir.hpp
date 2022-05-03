class IIR {
    public:
        float alpha;
        float y_prev;
        IIR(float a, float y0=0) {
            alpha = a;
            y_prev = y0;
        }

        float step(float input) {
            float y_n = alpha * y_prev + (1 - alpha) * input;
            y_prev = y_n;
            return y_n;
        }

        void set(float new_val) {
            y_prev = new_val;
        }

        void reset() {
            y_prev = 0;
        }
};