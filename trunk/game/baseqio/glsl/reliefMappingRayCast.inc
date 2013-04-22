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
    vec4 tHeight= texture2D(heightMap,tc);
    vec2 delta = vec2(-ray.x,-ray.y) * RELIEF_STEP_DELTA/ (ray.z);
    
    for(int i=1; i<=numsteps;i++)
    {      
        if(tHeight.x <= height)     
        {
            height = 1.0 - step*(i);
            currStep=i;
            tHeight= texture2D(heightMap,tc-delta*height);
        }
    }

    currStep-=1;
    height +=step; 

    for(int i=0; i<substeps;i++)
    {     
        step*=0.5;        
        height-=step; 
        tHeight= texture2D(heightMap,tc-delta*height); 
    
        if(tHeight.x >= height)
        {   
            height+=step;              
        }
    }
    return tc-delta*height;    
}