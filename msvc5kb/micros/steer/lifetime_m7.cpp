void Dead1(int);
void Dead2(int);
void Live3(int);
int __declspec(dllimport) __stdcall DeadImp(int);
int F1(int w) { if (0) Dead1(w); return w + 1; }
int F2(int w) { int k = 0; if (k) Dead2(w); return w + 2; }
int F3(int w) { if (w != w) DeadImp(w); Live3(w); return w + 3; }
