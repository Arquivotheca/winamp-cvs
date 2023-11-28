#ifdef LASER
typedef struct
{
  float x1, y1;
  float x2, y2;
  int color;
  int mode;
} LineType;

class C_LineListBase
{
	public:
    C_LineListBase(){ };
    virtual ~C_LineListBase() { };

    virtual int GetMaxLines(void)=0;
    virtual int GetUsedLines(void)=0;
    virtual void SetMaxLines(int m)=0;
    virtual void SetUsedLines(int usedlines)=0;
    virtual void SetLines(LineType *list, int start, int length)=0;
    virtual void ClearLineList(void)=0;
    virtual LineType *GetLineList(void)=0;
    virtual void AddLine(LineType *line)=0;
};

C_LineListBase *createLineList(void);
extern C_LineListBase *g_laser_linelist;
#endif