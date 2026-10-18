struct Move { short type; char rest[46]; };
extern int Sort(int type);
int TargetCalls(Move *objects, int i)
{
    return Sort(objects[i].type) > Sort(objects[i + 1].type);
}
