#version 330 core

	in		vec2	  v_position;		//the texture coordinate for the vertex
	in		mat3	  v_vertTBN;		//the vertex tbn matrix

	uniform	float	  u_starThreshold;	//the threshold for the stars - anything above this value will be made a star

			vec3	  color;			//intermediary color
			vec2	  position;			//intermediary position
			float	  starValue;		//the value of the noise field at the given coordinate

	out		vec3	  o_color;			//the color to pass to the renderer

	
//randomize a number (found online)
float hash(float n)
{
	return fract((1.0 + cos(n)) * 415.92653);
}

//generate random noise at the given coordinate
float noise(in vec2 position)
{
    float xhash = hash(position.x * 37.0);
    float yhash = hash(position.y * 57.0);

    return fract(xhash + yhash);
}

void main()
{
//base space-like color
    color = vec3(0.1, 0.1, 0.13);

    position = v_position + floor(v_vertTBN[0].xy);
	
//get the value of the noise field at the coordinate, and test to see if it is above the given threshold
    starValue = noise(position);
    starValue = pow((max(starValue - u_starThreshold, 0.0)) / (1.0 - u_starThreshold), 6.0);

//add the value to the color
	color += vec3(starValue);

//pass the color to the renderer
	o_color = color;
}