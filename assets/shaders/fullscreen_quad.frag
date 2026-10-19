#version 430 core

out vec4 fragColor;
in vec2 texCoords;

uniform sampler2D screenTex;
uniform sampler2D bloomTex;

void main()
{
/// ##############################################################################################
///                                     Common parameters
/// ##############################################################################################
        vec2 uv = texCoords;
        vec2 screenSize = vec2(textureSize(screenTex, 0));

        ivec2 texel = ivec2(uv * screenSize);
        int kernelSize = 2;

        
/// ##############################################################################################
///                                     Cross blur
/// ##############################################################################################
        // vec4 avg;
        // for(int x = -kernelSize; x <= kernelSize; x++)
        // {
        //         avg += texture(screenTex, (texel + vec2(x, 0)) / screenSize);
        // }

        // for(int y = -kernelSize; y <= kernelSize; y++)
        // {
        //         avg += texture(screenTex, (texel + vec2(0, y)) / screenSize);   
        // }

        // fragColor = avg / (kernelSize * 4);

        //fragColor = texture(screenTex, uv);


/// ##############################################################################################
///                                     Kuwahara Filter
/// ##############################################################################################
        vec4 sum = vec4(0.0);
        float sumSquared = 0.0;
        int count = 0;

        float minVariance = 1e30;
        vec4 bestColor = vec4(0.0);

        for(int r = 0; r < 4; r++)
        {
                bool right  = (r & 1) != 0;
                bool bottom = (r & 2) != 0;

                int x0 = right  ? 0 : -kernelSize;
                int x1 = right  ? kernelSize : 0;
                int y0 = bottom ? 0 : -kernelSize;
                int y1 = bottom ? kernelSize : 0;

                for (int y = y0; y <= y1; y++)
                {
                        for (int x = x0; x <= x1; x++)
                        {
                                vec4 color = texelFetch(screenTex, texel + ivec2(x, y), 0);

                                sum += color;
                                sumSquared += dot(color, color);
                                count++;
                        }
                }       
                vec4 mean = sum / float(count);

                float variance =
                sumSquared / float(count) - dot(mean, mean);

                variance = max(variance, 0.0);

                if (variance < minVariance)
                {
                        minVariance = variance;
                        bestColor = mean;
                }
        }

        const float bloomStrength = 0.04;
        vec3 sceneColor = texture(screenTex, texCoords).rgb;
        vec3 bloomColor = texture(bloomTex, texCoords).rgb;
        fragColor = vec4(mix(sceneColor, bloomColor, bloomStrength), 1.0);
}