/**
 * @file
 * @brief Source file for Saturation class
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @section LICENSE
 *
 * Copyright (c) 2008-2014 OpenShot Studios, LLC
 * <http://www.openshotstudios.com/>. This file is part of
 * OpenShot Library (libopenshot), an open-source project dedicated to
 * delivering high quality video editing and animation solutions to the
 * world. For more information visit <http://www.openshot.org/>.
 *
 * OpenShot Library (libopenshot) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpenShot Library (libopenshot) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../../include/effects/Saturation.h"

using namespace openshot;

/// Blank constructor, useful when using Json to load the effect properties
Saturation::Saturation() {
	// Init effect properties
	init_effect_details();

	// Init curves
	saturation = Keyframe(1.0);
}

// Default constructor
Saturation::Saturation(Keyframe new_saturation) : saturation(new_saturation)
{
	// Init effect properties
	init_effect_details();
}

// Init effect settings
void Saturation::init_effect_details()
{
	/// Initialize the values of the EffectInfo struct.
	InitEffectInfo();

	/// Set the effect info
	info.class_name = "Saturation";
	info.name = "Color Saturation";
	info.description = "Adjust the color saturation.";
	info.has_audio = false;
	info.has_video = true;
}

// Constrain a color value from 0 to 255
int Saturation::constrain(int color_value)
{
	// Constrain new color from 0 to 255
	if (color_value < 0)
		color_value = 0;
	else if (color_value > 255)
		color_value = 255;

	return color_value;
}

// This method is required for all derived classes of EffectBase, and returns a
// modified openshot::Frame object
tr1::shared_ptr<Frame> Saturation::GetFrame(tr1::shared_ptr<Frame> frame, long int frame_number)
{
	// Get the frame's image
	tr1::shared_ptr<QImage> frame_image = frame->GetImage();

	// Constants used for color saturation formula
	double pR = .299;
	double pG = .587;
	double pB = .114;

	// Loop through pixels
	unsigned char *pixels = (unsigned char *) frame_image->bits();
	for (int pixel = 0, byte_index=0; pixel < frame_image->width() * frame_image->height(); pixel++, byte_index+=4)
	{
		// Get the RGB values from the pixel
		int R = pixels[byte_index];
		int G = pixels[byte_index + 1];
		int B = pixels[byte_index + 2];
		int A = pixels[byte_index + 3];

		// Calculate the saturation multiplier
		double p = sqrt( (R * R * pR) +
						 (G * G * pG) +
						 (B * B * pB) );

		// Adjust the saturation
		R = p + (R - p) * saturation.GetValue(frame_number);
		G = p + (G - p) * saturation.GetValue(frame_number);
		B = p + (B - p) * saturation.GetValue(frame_number);

		// Constrain the value from 0 to 255
		R = constrain(R);
		G = constrain(G);
		B = constrain(B);

		// Set all pixels to new value
		pixels[byte_index] = R;
		pixels[byte_index + 1] = G;
		pixels[byte_index + 2] = B;
		pixels[byte_index + 3] = A; // leave the alpha value alone
	}

	// return the modified frame
	return frame;
}

// Generate JSON string of this object
string Saturation::Json() {

	// Return formatted string
	return JsonValue().toStyledString();
}

// Generate Json::JsonValue for this object
Json::Value Saturation::JsonValue() {

	// Create root json object
	Json::Value root = EffectBase::JsonValue(); // get parent properties
	root["type"] = info.class_name;
	root["saturation"] = saturation.JsonValue();

	// return JsonValue
	return root;
}

// Load JSON string into this object
void Saturation::SetJson(string value) throw(InvalidJSON) {

	// Parse JSON string into JSON objects
	Json::Value root;
	Json::Reader reader;
	bool success = reader.parse( value, root );
	if (!success)
		// Raise exception
		throw InvalidJSON("JSON could not be parsed (or is invalid)", "");

	try
	{
		// Set all values that match
		SetJsonValue(root);
	}
	catch (exception e)
	{
		// Error parsing JSON (or missing keys)
		throw InvalidJSON("JSON is invalid (missing keys or invalid data types)", "");
	}
}

// Load Json::JsonValue into this object
void Saturation::SetJsonValue(Json::Value root) {

	// Set parent data
	EffectBase::SetJsonValue(root);

	// Set data from Json (if key is found)
	if (!root["saturation"].isNull())
		saturation.SetJsonValue(root["saturation"]);
}

// Get all properties for a specific frame
string Saturation::PropertiesJSON(long int requested_frame) {

	// Requested Point
	Point requested_point(requested_frame, requested_frame);

	// Generate JSON properties list
	Json::Value root;
	root["id"] = add_property_json("ID", 0.0, "string", Id(), false, 0, -1, -1, CONSTANT, -1, true);
	root["position"] = add_property_json("Position", Position(), "float", "", false, 0, 0, 1000 * 60 * 30, CONSTANT, -1, false);
	root["layer"] = add_property_json("Layer", Layer(), "int", "", false, 0, 0, 1000, CONSTANT, -1, false);
	root["start"] = add_property_json("Start", Start(), "float", "", false, 0, 0, 1000 * 60 * 30, CONSTANT, -1, false);
	root["end"] = add_property_json("End", End(), "float", "", false, 0, 0, 1000 * 60 * 30, CONSTANT, -1, false);
	root["duration"] = add_property_json("Duration", Duration(), "float", "", false, 0, 0, 1000 * 60 * 30, CONSTANT, -1, true);

	// Keyframes
	root["saturation"] = add_property_json("Saturation", saturation.GetValue(requested_frame), "float", "", saturation.Contains(requested_point), saturation.GetCount(), -10000, 10000, saturation.GetClosestPoint(requested_point).interpolation, saturation.GetClosestPoint(requested_point).co.X, false);

	// Return formatted string
	return root.toStyledString();
}

