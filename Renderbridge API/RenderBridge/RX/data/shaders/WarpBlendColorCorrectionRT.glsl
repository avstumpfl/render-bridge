
// RGB to HSV conversion
vec3 rgb2hsv(vec3 rgb) {
    float Cmax = max(rgb.r, max(rgb.g, rgb.b));
    float Cmin = min(rgb.r, min(rgb.g, rgb.b));
    float delta = Cmax - Cmin;
    
    vec3 hsv = vec3(0.0, 0.0, Cmax);
    
    if (delta != 0.0) {
        if (Cmax == rgb.r) {
            hsv.x = 60.0 * mod((rgb.g - rgb.b) / delta, 6.0);
        } else if (Cmax == rgb.g) {
            hsv.x = 60.0 * ((rgb.b - rgb.r) / delta + 2.0);
        } else {
            hsv.x = 60.0 * ((rgb.r - rgb.g) / delta + 4.0);
        }
        
        hsv.y = (Cmax != 0.0) ? (delta / Cmax) : 0.0;
    }
    
    return hsv;
}

// HSV to RGB conversion
vec3 hsv2rgb(vec3 hsv) {
    float h = hsv.x / 60.0;
    float s = hsv.y;
    float v = hsv.z;
    
    float c = v * s;
    float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
    float m = v - c;
    
    vec3 rgb;
    
    if (h < 1.0) rgb = vec3(c, x, 0.0);
    else if (h < 2.0) rgb = vec3(x, c, 0.0);
    else if (h < 3.0) rgb = vec3(0.0, c, x);
    else if (h < 4.0) rgb = vec3(0.0, x, c);
    else if (h < 5.0) rgb = vec3(x, 0.0, c);
    else rgb = vec3(c, 0.0, x);
    
    return rgb + m;
}

vec3 kelvinToRGB(float temperature) 
{
    // Normalize temperature around 6500K as neutral point
    float temp = clamp(temperature, 1000.0, 12000.0);
    
    vec3 color;
    
    if(temp <= 6500.0) {
        // From 1000K to 6500K
        float t = (temp - 1000.0) / 5500.0; // 0->1 range
        
        color.r = 1.0;                          // Red stays at 1.0
        color.g = 0.39 * pow(t, 0.33) + 0.61;  // Green increases smoothly to 1.0
        color.b = 0.47 * pow(t, 1.5) + 0.53;   // Blue increases more dramatically
    }
    else {
        // From 6500K to 12000K  
        float t = (temp - 6500.0) / 5500.0; // 0->1 range
        
        color.r = 1.0 - (0.23 * t);         // Red decreases slightly
        color.g = 1.0;                      // Green stays at 1.0
        color.b = 1.0;                      // Blue stays at 1.0
    }
    
    return color;
}



void color_correction_realtime(inout vec4 color, mat4 color_parameters) {
  // Apply shader only if not receiving default values.
  // Flag for default values is set in color_parameters[3][3].
  if (color_parameters[3][3] > 0)
  {
    vec3 rgb = color.rgb;
    
    // Convert to HSV
    vec3 hsv = rgb2hsv(rgb);
    
    // Apply HSV adjustments
    hsv.x = mod(hsv.x + color_parameters[0][0], 360.0);  // Hue
    hsv.y *= color_parameters[0][1];                     // Saturation
    hsv.z *= color_parameters[0][2];                     // Value
    
    // Convert back to RGB
    rgb = hsv2rgb(hsv);
    
    // Apply RGB multipliers
    rgb.r *= color_parameters[1][0];
    rgb.g *= color_parameters[1][1];
    rgb.b *= color_parameters[1][2];
    
    // Apply brightness
    rgb *= color_parameters[2][0];
    
    // Apply contrast
    rgb = (rgb - 0.5) * color_parameters[2][1] + 0.5;
    
    // Apply gamma
    rgb = pow(rgb, vec3(2.2 / color_parameters[2][2]));
    
    // Apply color temperature
    rgb *= kelvinToRGB(color_parameters[2][3]);
    
    // Output final color
    color = vec4(rgb, color.a);
  }
}
