#version 450 core

#define PI 3.14159265358979323846
out vec4 FragColor;
in  vec2 texCoords;

uniform sampler2D screenTexture;
uniform vec2 screenSize;
uniform bool  grayscale, negative, vignette, bloom, blur, toonShading;
uniform float vignetteIntensity, bloomIntensity, bloomThreshold;
uniform int   bloomSpread, blurRadius, toonLevels;

vec4 RGBAtoHSVA(vec4 rgba)
{
    vec4 hsva;

    float minV = min(min(rgba.r, rgba.g), rgba.b);
    float maxV = max(max(rgba.r, rgba.g), rgba.b);
    float diff = maxV - minV;

    float r = rgba.r;
    float g = rgba.g;
    float b = rgba.b;

    // Set Value.
    hsva.b = maxV;

    // If max and min are the same, return.
    if (diff < 0.00001)
        return vec4(0, 0, hsva.b, rgba.a);

    // Set Saturation.
    if (maxV > 0)
        hsva.g = diff / maxV;
    else
        return vec4(0, 0, hsva.b, rgba.a);

    // Set Hue.
    if (r >= maxV)
        hsva.r = (g - b) / diff;
    else if (g >= maxV)
        hsva.r = 2.0f + (b - r) / diff;
    else if (b >= maxV)
        hsva.r = 4.0f + (r - g) / diff;

    // Keep Hue above 0.
    if (hsva.r < 0)
        hsva.r += 2 * PI;

    hsva.a = rgba.a;
    return hsva;
}

vec4 HSVAtoRGBA(vec4 hsva)
{
    vec4 rgba = vec4(0, 0, 0, hsva.a);

    // Red channel
    float k = mod((5.0f + hsva.r), 6);
    float t = 4.0f - k;
    k = (t < k) ? t : k;
    k = (k < 1) ? k : 1;
    k = (k > 0) ? k : 0;
    rgba.r = hsva.b - hsva.b * hsva.g * k;

    // Green channel
    k = mod((3.0f + hsva.r), 6);
    t = 4.0f - k;
    k = (t < k) ? t : k;
    k = (k < 1) ? k : 1;
    k = (k > 0) ? k : 0;
    rgba.g = hsva.b - hsva.b * hsva.g * k;

    // Blue channel
    k = mod((1.0f + hsva.r), 6);
    t = 4.0f - k;
    k = (t < k) ? t : k;
    k = (k < 1) ? k : 1;
    k = (k > 0) ? k : 0;
    rgba.b = hsva.b - hsva.b * hsva.g * k;

    return rgba;
}

void ApplyGrayscale()
{
    vec4 hsva = RGBAtoHSVA(FragColor);
    FragColor = vec4(hsva.b, hsva.b, hsva.b, FragColor.a);
}

void ApplyNegative()
{
    FragColor = 1.0 - FragColor * 0.5;
}

void ApplyVignette()
{
    vec2 uv = texCoords;
    uv *=  1.0 - uv.yx;
    float vig = uv.x*uv.y * 15.0;
    vig = pow(vig, vignetteIntensity);
    FragColor *= vec4(vig, vig, vig, FragColor.a); 
}

void ApplyBloom()
{
    float x, y, xx, yy, r=bloomSpread, rr=r*r, dx, dy, w, w0;
    w0 = 0.3780 / pow(r, 1.975);
    vec2 p;
    vec4 col = vec4(0.0);
    for (dx = 1.0/screenSize.x, x = -r, p.x = texCoords.x + (x*dx); x <= r; x++, p.x += dx)
    { 
        xx = x*x;
        for (dy = 1.0/screenSize.y, y = -r, p.y = texCoords.y + (y*dy); y <= r; y++, p.y += dy)
        { 
            yy = y*y;
            if (xx + yy <= rr)
            {
                vec4 curPixel = texture(screenTexture, p);
                if ((curPixel.r + curPixel.g + curPixel.b) / 3 > bloomThreshold)
                {
                    w = w0 * exp((-xx-yy) / (2.0*rr));
                    col += curPixel * w;
                }
            }
        }
    }
    FragColor += col * bloomIntensity;
}

void ApplyBlur()
{
    float x, y, xx, yy, r=blurRadius, rr=r*r, dx, dy, w, w0;
    w0 = 0.3780 / pow(r, 1.975);
    vec2 p;
    vec4 col = vec4(0.0);
    for (dx = 1.0/screenSize.x, x = -r, p.x = texCoords.x + (x*dx); x <= r; x++, p.x += dx)
    { 
        xx = x*x;
        for (dy = 1.0/screenSize.y, y = -r, p.y = texCoords.y + (y*dy); y <= r; y++, p.y += dy)
        { 
            yy = y*y;
            if (xx + yy <= rr)
            {
                w = w0 * exp((-xx-yy) / (2.0*rr));
                col += texture(screenTexture, p) * w;
            }
        }
    }
    FragColor = col;
}

void ApplyToonShading()
{
    vec4 hsva = RGBAtoHSVA(FragColor);
    hsva.b = round(hsva.b * toonLevels) / toonLevels;
    FragColor = HSVAtoRGBA(hsva);
}

void main()
{
    FragColor = texture(screenTexture, texCoords);
    if (bloom)       ApplyBloom();
    if (blur)        ApplyBlur();
    if (vignette)    ApplyVignette();
    if (toonShading) ApplyToonShading();
    if (grayscale)   ApplyGrayscale();
    if (negative)    ApplyNegative();
}
