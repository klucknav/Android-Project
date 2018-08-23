uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform float shininess;
varying vec3 normal, eyeVec, lightDir;

void main(){
    vec4 blueColor = vec4(0.3, 0.8, 0.8, 1.0);
    vec4 lightAmbient = vec4(0.5, 0.5, 0.5, 1.0);
    vec4 diffuse;
    vec4 specularColor;
    //vec4 finalColor = vec4(1.0, .0,.0,1.0);
    vec3 N = normalize(normal);
    vec3 L = normalize(lightDir);
    float lambert = dot(N,L);

    if (lambert > 0.0) {
        diffuse = lightDiffuse * lambert;
        vec3 E = normalize(eyeVec);
        vec3 R = reflect(-L, N);
        float specular = pow(max(dot(R, E), 0.0), shininess);
        specularColor = lightSpecular * specular;
    }
    gl_FragColor = blueColor * (lightAmbient + diffuse + specularColor);
}