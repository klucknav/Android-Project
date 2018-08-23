varying vec4 color;
const vec3 lightPos = vec3(0,-1,5);
const vec4 lightAmbient = vec4(0.3, 0.3, 0.3, 1.0);
const vec4 lightDiffuse = vec4(0.4, 0.4, 0.4, 1.0);
const vec4 lightSpecular = vec4(0.8, 0.8, 0.8, 1.0);

const vec4 cessnaColor = vec4(0.8, 0.8, 0.8, 1.0);
const vec4 blueColor = vec4(0.3, 0.8, 0.8, 1.0);

void DirectionalLight(in vec3 normal, in vec3 ecPos,
                    inout vec4 ambient, inout vec4 diffuse, inout vec4 specular){
    float nDotVP;
    vec3 L = normalize(gl_ModelViewMatrix*vec4(lightPos, 0.0)).xyz;
    nDotVP = max(0.0, dot(normal, L));

    if (nDotVP > 0.0) {
        vec3 E = normalize(-ecPos);
        vec3 R = normalize(reflect( L, normal ));
        specular = pow(max(dot(R, E), 0.0), 16.0) * lightSpecular;
    }
    ambient  = lightAmbient;
    diffuse  = lightDiffuse * nDotVP;
}

void main() {
    vec4 ambiCol = vec4(0.0);
    vec4 diffCol = vec4(0.0);
    vec4 specCol = vec4(0.0);
    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;
    vec3 normal   = normalize(gl_NormalMatrix * gl_Normal);
    vec4 ecPos    = gl_ModelViewMatrix * gl_Vertex;
    DirectionalLight(normal, ecPos.xyz, ambiCol, diffCol, specCol);
    color = blueColor * (ambiCol + diffCol + specCol);
}