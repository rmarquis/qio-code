#define RELIEF_STEP_COUNT 10
#define RELIEF_SUBSTEP_COUNT 5
#define RELIEF_STEP_DELTA 0.05

vec2 ReliefMappingRayCast(vec2 tc, vec3 ray)
{
    const int numsteps = RELIEF_STEP_COUNT;
    const int substeps = RELIEF_SUBSTEP_COUNT;   
 
    float height = 1.0;
    float step = 1.0/float(numsteps);
    int currStep;
#ifdef HAS_BUMP_HEIGHTMAP_MAP
    float tHeight = texture2D(bumpMap,tc).a;
#else
    float tHeight = texture2D(heightMap,tc).a;
#endif
    vec2 delta = vec2(-ray.x,-ray.y) * RELIEF_STEP_DELTA/ (ray.z);
    
    for(int i=1; i<=numsteps;i++)
    {      
        if(tHeight.x <= height)     
        {
            height = 1.0 - step*(i);
            currStep=i;
#ifdef HAS_BUMP_HEIGHTMAP_MAP
            tHeight = texture2D(bumpMap,tc-delta*height).a;
#else
            tHeight = texture2D(heightMap,tc-delta*height);
#endif
        }
    }

    currStep-=1;
    height +=step; 

    for(int i=0; i<substeps;i++)
    {     
        step*=0.5;        
        height-=step; 
#ifdef HAS_BUMP_HEIGHTMAP_MAP
        tHeight= texture2D(bumpMap,tc-delta*height).a;
#else
        tHeight= texture2D(heightMap,tc-delta*height);
#endif 
    
        if(tHeight.x >= height)
        {   
            height+=step;              
        }
    }
    return tc-delta*height;    
}