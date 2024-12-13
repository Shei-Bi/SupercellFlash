#pragma once

#include <flash/flash.h>
#include <filesystem>

#include <core/stb/stb.h>

#include <nlohmann/json.hpp>

#include "SpriteData.hpp"

using namespace sc::flash;
using json = nlohmann::ordered_json;

namespace sc
{
	namespace sctex
	{
		bool CompareUV(float x, float y, float epsilon = 0.001f) {
			if (fabs(x - y) < epsilon)
				return true;
			return false;
		}

		bool CommandEqual(const ShapeDrawBitmapCommand& a, const ShapeDrawBitmapCommand& b)
		{
			if (a.texture_index != b.texture_index) return false;
			if (a.vertices.size() != b.vertices.size()) return false;

			for (uint16_t i = 0; a.vertices.size() > i; i++)
			{
				const ShapeDrawBitmapCommandVertex& vertex_a = a.vertices[i];
				const ShapeDrawBitmapCommandVertex& vertex_b = b.vertices[i];

				if (!CompareUV(vertex_a.u, vertex_b.u)) return false;
				if (!CompareUV(vertex_a.v, vertex_b.v)) return false;
			}

			return  true;
		}

		void DecodePixel(uint8_t* pixel_data, const wk::Image::PixelDepthInfo& pixel_info, uint8_t& r_channel, uint8_t& g_channel, uint8_t& b_channel, uint8_t& a_channel)
		{
			uint32_t pixel_buffer = 0;

			wk::Memory::copy(
				pixel_data,
				(uint8_t*)(&pixel_buffer),
				pixel_info.byte_count
			);

			{
				uint8_t bit_index = 0;

				if (pixel_info.r_bits != 0xFF)
				{
					uint64_t r_bits_mask = (uint64_t)pow(2, pixel_info.r_bits) - 1;
					r_channel = static_cast<uint8_t>(r_bits_mask & pixel_buffer);

					bit_index += pixel_info.r_bits;
				}

				if (pixel_info.g_bits != 0xFF)
				{
					uint64_t g_bits_mask = (uint64_t)pow(2, pixel_info.g_bits) - 1;
					g_channel = static_cast<uint8_t>(((g_bits_mask << bit_index) & pixel_buffer) >> bit_index);

					bit_index += pixel_info.g_bits;
				}
				else
				{
					g_channel = r_channel;
				}

				if (pixel_info.b_bits != 0xFF)
				{
					uint64_t b_bits_mask = (uint64_t)pow(2, pixel_info.b_bits) - 1;
					b_channel = static_cast<uint8_t>(((b_bits_mask << bit_index) & pixel_buffer) >> bit_index);

					bit_index += pixel_info.b_bits;
				}
				else
				{
					b_channel = g_channel;
				}

				if (pixel_info.a_bits != 0xFF)
				{
					uint64_t a_bits_mask = (uint64_t)pow(2, pixel_info.a_bits) - 1;
					a_channel = static_cast<uint8_t>(((a_bits_mask << bit_index) & pixel_buffer) >> bit_index);
				}
				else
				{
					a_channel = 0xFF;
				}
			}
		}

		void EncodePixel(uint8_t* pixel_data, const wk::Image::PixelDepthInfo& pixel_info, const uint8_t r_channel, const uint8_t g_channel, const uint8_t b_channel, const uint8_t a_channel)
		{
			uint32_t pixel_buffer = 0;

			{
				uint8_t bit_index = pixel_info.byte_count * 8;

				if (pixel_info.a_bits != 0xFF)
				{
					bit_index -= pixel_info.a_bits;
					uint64_t bits_mask = (uint64_t)((pow(2, pixel_info.a_bits) - 1));
					pixel_buffer |= ((a_channel & bits_mask)) << bit_index;
				}

				if (pixel_info.b_bits != 0xFF)
				{
					bit_index -= pixel_info.b_bits;
					uint64_t bits_mask = (uint64_t)((pow(2, pixel_info.b_bits) - 1));
					pixel_buffer |= ((b_channel & bits_mask)) << bit_index;
				}

				if (pixel_info.g_bits != 0xFF)
				{
					bit_index -= pixel_info.g_bits;
					uint64_t bits_mask = (uint64_t)((pow(2, pixel_info.g_bits) - 1));
					pixel_buffer |= ((g_channel & bits_mask)) << bit_index;
				}

				if (pixel_info.r_bits != 0xFF)
				{
					bit_index -= pixel_info.r_bits;
					uint64_t bits_mask = (uint64_t)((pow(2, pixel_info.r_bits) - 1));
					pixel_buffer |= ((r_channel & bits_mask)) << bit_index;
				}
			}

			wk::Memory::copy(
				(uint8_t*)&pixel_buffer,
				pixel_data,
				pixel_info.byte_count
			);
		}

		void PremultiplyToStraight(wk::RawImage& image)
		{
			uint64_t pixel_count = image.width() * image.height();

			const wk::Image::PixelDepthInfo& pixel_info = wk::Image::PixelDepthTable[(uint16_t)image.depth()];

			for (uint64_t pixel_index = 0; pixel_count > pixel_index; pixel_index++)
			{
				uint8_t* pixel_data = image.data() + (pixel_index * pixel_info.byte_count);

				uint8_t r_channel = 0;
				uint8_t g_channel = 0;
				uint8_t b_channel = 0;
				uint8_t a_channel = 0;

				DecodePixel(pixel_data, pixel_info, r_channel, g_channel, b_channel, a_channel);

				if (a_channel == 0) continue;

				// PreAlpha to Straight
				float factor = ((float)a_channel / 255.f);
				r_channel = (uint8_t)(std::clamp(r_channel / factor, 0.f, 255.f));
				g_channel = (uint8_t)(std::clamp(g_channel / factor, 0.f, 255.f));
				b_channel = (uint8_t)(std::clamp(b_channel / factor, 0.f, 255.f));

				EncodePixel(pixel_data, pixel_info, r_channel, g_channel, b_channel, a_channel);
			}
		}

		void StraightToPremultiply(wk::RawImage& image)
		{
			uint64_t pixel_count = image.width() * image.height();

			const wk::Image::PixelDepthInfo& pixel_info = wk::Image::PixelDepthTable[(uint16_t)image.depth()];

			for (uint64_t pixel_index = 0; pixel_count > pixel_index; pixel_index++)
			{
				uint8_t* pixel_data = image.data() + (pixel_index * pixel_info.byte_count);

				uint8_t r_channel = 0;
				uint8_t g_channel = 0;
				uint8_t b_channel = 0;
				uint8_t a_channel = 0;

				DecodePixel(pixel_data, pixel_info, r_channel, g_channel, b_channel, a_channel);

				if (a_channel == 0)
				{
					r_channel = 0;
					g_channel = 0;
					b_channel = 0;
				}
				else
				{
					//  Straight to PreAlpha
					float factor = ((float)a_channel / 255.f);
					r_channel = (uint8_t)((float)r_channel * factor);
					g_channel = (uint8_t)((float)g_channel * factor);
					b_channel = (uint8_t)((float)b_channel * factor);
				}

				EncodePixel(pixel_data, pixel_info, r_channel, g_channel, b_channel, a_channel);
			}
		}
	}
}

namespace sc
{
	namespace sctex
	{
		class SWFFile : public SupercellSWF
		{
		public:
			SWFFile() {};

			SWFFile(std::filesystem::path path, bool load_all = false)
			{
				current_file = path;

				wk::InputFileStream file(path);
				if (SupercellSWF::IsSC2(file))
				{
					std::cout << "File is loaded as a SC2" << std::endl;
					load_sc2(file);
				}
				else
				{
					load_sc1(path, load_all);
				}
			}

		public:
			// If the file is a real texture, only the texture tags are loaded, otherwise the entire file is loaded.
			void load_sc1(std::filesystem::path path, bool load_all = false)
			{
				stream.open_file(path);

				// Path Check
				if (path.string().find("_tex") != std::string::npos)
				{
					load_texures_from_binary();

					return;
				}

				// First tag check
				{
					uint8_t tag = stream.read_unsigned_byte();
					int32_t tag_length = stream.read_int();

					switch (tag)
					{
					case TAG_TEXTURE:
					case TAG_TEXTURE_2:
					case TAG_TEXTURE_3:
					case TAG_TEXTURE_4:
					case TAG_TEXTURE_5:
					case TAG_TEXTURE_6:
					case TAG_TEXTURE_7:
					case TAG_TEXTURE_8:
					case TAG_TEXTURE_9:
						if (tag_length <= 0) break;

						std::cout << "File is loaded as a texture file because the first tag is a texture" << std::endl;

						stream.seek(0);
						load_texures_from_binary();

						return;
					default:
						stream.seek(0);
						break;
					}
				}

				// Whole file loading
				{
					std::cout << "File is loaded as a default asset file because it did not pass all texture checks" << std::endl;

					if (load_all)
					{
						stream.clear();
						load(path);
					}
					else
					{
						// Skip of all count fields
						stream.seek(17);

						// export names skip
						uint16_t exports_count = stream.read_unsigned_short();

						for (uint16_t i = 0; exports_count > i; i++)
						{
							stream.read_unsigned_short();
						}

						for (uint16_t i = 0; exports_count > i; i++)
						{
							uint8_t length = stream.read_unsigned_byte();
							stream.seek(length, wk::Stream::SeekMode::Add);
						}

						load_texures_from_binary();
					}
				}
			}

			void load_texures_from_binary()
			{
				while (true)
				{
					uint8_t tag = stream.read_unsigned_byte();
					int32_t tag_length = stream.read_int();

					if (tag == TAG_END)
						break;

					if (tag_length < 0)
						throw wk::Exception("Negative tag length");

					switch (tag)
					{
					case TAG_TEXTURE:
					case TAG_TEXTURE_2:
					case TAG_TEXTURE_3:
					case TAG_TEXTURE_4:
					case TAG_TEXTURE_5:
					case TAG_TEXTURE_6:
					case TAG_TEXTURE_7:
					case TAG_TEXTURE_8:
					case TAG_TEXTURE_9:
					case TAG_TEXTURE_10:
						textures.emplace_back().load(*this, tag, true);
						break;

					default:
						stream.seek(tag_length, wk::Stream::SeekMode::Add);
						break;
					}
				}
			}
			static void pasteTex(uint8_t* thisData, uint8_t* otherData, int otherWidth, int otherHeight, int x, int y, int width, int height) {
				for (int i = 0;i < height;i++) {
					for (int j = 0;j < width;j++) {
						for (int k = 0;k < 4;k++) {
							thisData[(i * width + j) * 4 + k] = otherData[((i + y) * otherWidth + (j + x)) * 4 + k];
							// std::cout << ((i * height + j) * 4 + k) << std::endl;
						}
					}
				}
			}
			void save_sprites_to_folder(std::filesystem::path output_path)
			{
				wk::RawImage** images = new wk::RawImage * [textures.size()];
				uint8_t** buckets = new uint8_t * [textures.size()];
				for (uint16_t i = 0; textures.size() > i; i++)
				{
					SWFTexture& texture = textures[i];

					// Texture Image
					// std::filesystem::path basename = output_path.stem();
					// std::filesystem::path output_image_path = output_path / basename.concat("_").concat(std::to_string(i)).concat(".png");
					// wk::OutputFileStream output_image(output_image_path);

					// wk::stb::ImageFormat format = wk::stb::ImageFormat::PNG;

					switch (texture.encoding())
					{
					case SWFTexture::TextureEncoding::KhronosTexture:
					{
						wk::RawImage image(
							texture.image()->width(), texture.image()->height(),
							texture.image()->depth()
						);

						wk::SharedMemoryStream image_data(image.data(), image.data_length());
						((sc::texture::KhronosTexture*)(texture.image()))->decompress_data(image_data);

						PremultiplyToStraight(image);
						// wk::stb::write_image(image, format, output_image);
						images[i] = &image;
						buckets[i] = wk::Memory::allocate(wk::Image::calculate_image_length(images[i]->width(), images[i]->height(), wk::Image::PixelDepth::RGBA8));

						wk::Image::remap(
							images[i]->data(), buckets[i],
							images[i]->width(), images[i]->height(),
							images[i]->depth(), wk::Image::PixelDepth::RGBA8
						);
					}
					break;

					case SWFTexture::TextureEncoding::Raw:
					{
						texture.linear(true);

						wk::RawImage& image = *(wk::RawImage*)(texture.image());
						PremultiplyToStraight(image);
						// wk::stb::write_image(
						// 	image,
						// 	format,
						// 	output_image
						// );
						images[i] = &image;
					}
					break;

					default:
						break;
					}
					// std::cout << "Decoded texture: " << output_image_path << std::endl;
				}
				// for (int i = 0;i < textures.size();i++) {
				// 	buckets[i] = wk::Memory::allocate(wk::Image::calculate_image_length(images[i]->width(), images[i]->height(), wk::Image::PixelDepth::RGBA8));

				// 	wk::Image::remap(
				// 		images[i]->data(), buckets[i],
				// 		images[i]->width(), images[i]->height(),
				// 		images[i]->depth(), wk::Image::PixelDepth::RGBA8
				// 	);
				// }
				std::filesystem::path basename = output_path.stem();
				for (ExportName& exportName : exports) {
					DisplayObject* displayObject = &GetDisplayObjectByID(exportName.id);
					if (!displayObject->is_movieclip()) continue;
					MovieClip* movieClip = (MovieClip*)displayObject;
					if (movieClip->frames.size() != 1) continue;
					std::cout << exportName.name.string() << std::endl;
					for (DisplayObjectInstance& displayObjectInstance : movieClip->childrens) {
						DisplayObject* displayObject = &GetDisplayObjectByID(displayObjectInstance.id);
						if (!displayObject->is_shape()) continue;
						Shape* shape = (Shape*)displayObject;
						assert(shape->commands.size() == 1);
						wk::RectF bounds(0.0f, 0.0f, 0.0f, 0.0f);
						wk::RectF boundsTex(1.0f, 1.0f, 0.0f, 0.0f);
						for (ShapeDrawBitmapCommandVertex& vertex : shape->commands[0].vertices) {
							if (bounds.left > vertex.x) bounds.left = vertex.x;
							if (bounds.right < vertex.x) bounds.right = vertex.x;
							if (bounds.top > vertex.y) bounds.top = vertex.y;
							if (bounds.bottom < vertex.y) bounds.bottom = vertex.y;
							if (boundsTex.left > vertex.u) boundsTex.left = vertex.u;
							if (boundsTex.right < vertex.u) boundsTex.right = vertex.u;
							if (boundsTex.top > vertex.v) boundsTex.top = vertex.v;
							if (boundsTex.bottom < vertex.v) boundsTex.bottom = vertex.v;
						}
						int outWidth = bounds.right - bounds.left;
						int outHeight = bounds.bottom - bounds.top;
						int outLength = wk::Image::calculate_image_length(outWidth, outHeight, wk::Image::PixelDepth::RGBA8);
						auto buffer = wk::Memory::allocate(outLength);
						auto image = images[shape->commands[0].texture_index];
						pasteTex(buffer, buckets[shape->commands[0].texture_index], image->width(), image->height(), boundsTex.left * image->width(), boundsTex.top * image->height(), outWidth, outHeight);
						wk::RawImage out(buffer, outWidth, outHeight, wk::Image::PixelDepth::RGBA8, wk::Image::ColorSpace::Linear);
						wk::OutputFileStream output_image((basename / exportName.name.string()).concat(".png"));
						wk::stb::write_image(out, wk::stb::ImageFormat::PNG, output_image);
						// return;
					}
				}
			}

			void save_textures_to_folder(std::filesystem::path output_path)
			{
				json texture_infos = json::array();

				for (uint16_t i = 0; textures.size() > i; i++)
				{
					SWFTexture& texture = textures[i];

					// Texture Info
					{
						std::string encoding;
						{
							switch (texture.encoding())
							{
							case SWFTexture::TextureEncoding::KhronosTexture:
								encoding = "khronos";
								break;
							case SWFTexture::TextureEncoding::Raw:
								encoding = "raw";
								break;
							default:
								break;
							}
						}

						std::string pixel_type = "RGBA8";

						switch (texture.pixel_format())
						{
						case SWFTexture::PixelFormat::RGBA4:
							pixel_type = "RGBA4";
							break;
						case SWFTexture::PixelFormat::RGB5_A1:
							pixel_type = "RGB5_A1";
							break;
						case SWFTexture::PixelFormat::RGB565:
							pixel_type = "RGB565";
							break;
						case SWFTexture::PixelFormat::LUMINANCE8_ALPHA8:
							pixel_type = "LUMINANCE8_ALPHA8";
							break;
						case SWFTexture::PixelFormat::LUMINANCE8:
							pixel_type = "LUMINANCE8";
							break;

						case SWFTexture::PixelFormat::RGBA8:
						default:
							break;
						}

						std::string filtering = "LINEAR_NEAREST";

						switch (texture.filtering)
						{
						case SWFTexture::Filter::LINEAR_MIPMAP_NEAREST:
							filtering = "LINEAR_MIPMAP_NEAREST";
							break;
						case SWFTexture::Filter::NEAREST_NEAREST:
							filtering = "NEAREST_NEAREST";
							break;
						case SWFTexture::Filter::LINEAR_NEAREST:
						default:
							break;
						}

						json texture_info = {
							{"Encoding", encoding},
							{"PixelFormat", pixel_type},
							{"Filtering", filtering},
							{"Linear", texture.linear()},
						};

						texture_infos.push_back(texture_info);
					}

					// Texture Image
					std::filesystem::path basename = output_path.stem();
					std::filesystem::path output_image_path = output_path / basename.concat("_").concat(std::to_string(i)).concat(".png");
					wk::OutputFileStream output_image(output_image_path);

					wk::stb::ImageFormat format = wk::stb::ImageFormat::PNG;

					switch (texture.encoding())
					{
					case SWFTexture::TextureEncoding::KhronosTexture:
					{
						wk::RawImage image(
							texture.image()->width(), texture.image()->height(),
							texture.image()->depth()
						);

						wk::SharedMemoryStream image_data(image.data(), image.data_length());
						((sc::texture::KhronosTexture*)(texture.image()))->decompress_data(image_data);

						PremultiplyToStraight(image);
						wk::stb::write_image(image, format, output_image);
					}
					break;

					case SWFTexture::TextureEncoding::Raw:
					{
						texture.linear(true);

						wk::RawImage& image = *(wk::RawImage*)(texture.image());
						PremultiplyToStraight(image);
						wk::stb::write_image(
							image,
							format,
							output_image
						);
					}
					break;

					default:
						break;
					}

					std::cout << "Decoded texture: " << output_image_path << std::endl;
				}

				std::string serialized_data = texture_infos.dump(4);

				wk::OutputFileStream file_info(output_path / output_path.stem().concat(".json"));
				file_info.write(serialized_data.data(), serialized_data.size());
			}

			// void load_textures_from_folder(std::filesystem::path input)
			// {
			// 	current_file = input;
			// 	std::filesystem::path basename = input.stem();
			// 	std::filesystem::path texture_info_path = std::filesystem::path(input / basename.concat(".json"));

			// 	// Texture Info Parsing
			// 	json texture_infos = json::array({});

			// 	if (!std::filesystem::exists(texture_info_path))
			// 	{
			// 		std::cout << "Texture info file does not exist. Default settings will be used instead" << std::endl;
			// 	}
			// 	else
			// 	{
			// 		std::ifstream file(texture_info_path);
			// 		texture_infos = json::parse(file);
			// 	}

			// 	// Texture Images path gather
			// 	SWFVector<std::filesystem::path> texture_images_paths;

			// 	for (auto const& file_descriptor : std::filesystem::directory_iterator(input))
			// 	{
			// 		std::filesystem::path filepath = file_descriptor.path();
			// 		std::filesystem::path file_extension = filepath.extension();

			// 		if (file_extension == ".png")
			// 		{
			// 			texture_images_paths.push_back(filepath);
			// 		}
			// 	}

			// 	//Texture Converting
			// 	for (uint16_t i = 0; texture_images_paths.size() > i; i++)
			// 	{
			// 		// Image Loading
			// 		wk::Ref<wk::RawImage> image;
			// 		wk::InputFileStream image_file(texture_images_paths[i]);
			// 		wk::stb::load_image(image_file, image.);
			// 		StraightToPremultiply(*image);

			// 		wk::SharedMemoryStream image_data(image->data(), image->data_length());

			// 		// Image Converting
			// 		SWFTexture texture;
			// 		texture.load_from_image(*image);

			// 		if (texture_infos.size() > i)
			// 		{
			// 			json texture_info = texture_infos[i];

			// 			SWFTexture::PixelFormat texture_type = SWFTexture::PixelFormat::RGBA8;

			// 			if (texture_info["PixelFormat"] == "RGBA4")
			// 			{
			// 				texture_type = SWFTexture::PixelFormat::RGBA4;
			// 			}
			// 			else if (texture_info["PixelFormat"] == "RGB5_A1")
			// 			{
			// 				texture_type = SWFTexture::PixelFormat::RGB5_A1;
			// 			}
			// 			else if (texture_info["PixelFormat"] == "RGB565")
			// 			{
			// 				texture_type = SWFTexture::PixelFormat::RGB565;
			// 			}
			// 			else if (texture_info["PixelFormat"] == "LUMINANCE8_ALPHA8")
			// 			{
			// 				texture_type = SWFTexture::PixelFormat::LUMINANCE8_ALPHA8;
			// 			}
			// 			else if (texture_info["PixelFormat"] == "LUMINANCE8")
			// 			{
			// 				texture_type = SWFTexture::PixelFormat::LUMINANCE8;
			// 			}

			// 			if (texture_info["Filtering"] == "LINEAR_NEAREST")
			// 			{
			// 				texture.filtering = SWFTexture::Filter::LINEAR_NEAREST;
			// 			}
			// 			else if (texture_info["Filtering"] == "LINEAR_MIPMAP_NEAREST")
			// 			{
			// 				texture.filtering = SWFTexture::Filter::LINEAR_MIPMAP_NEAREST;
			// 			}
			// 			else if (texture_info["Filtering"] == "NEAREST_NEAREST")
			// 			{
			// 				texture.filtering = SWFTexture::Filter::NEAREST_NEAREST;
			// 			}

			// 			if (texture_info["Encoding"] == "khronos")
			// 			{
			// 				texture.encoding(SWFTexture::TextureEncoding::KhronosTexture);
			// 			}
			// 			else if (texture_info["Encoding"] == "raw")
			// 			{
			// 				texture.encoding(SWFTexture::TextureEncoding::Raw);
			// 				texture.pixel_format(texture_type);
			// 			}
			// 		}

			// 		// Texture Insert
			// 		if (textures.size() <= i)
			// 		{
			// 			textures.push_back(texture);
			// 		}
			// 		else
			// 		{
			// 			textures[i] = texture;
			// 		}

			// 		std::cout << "Processed texture: " << texture_images_paths[i] << std::endl;
			// 	}
			// }
		};
	}
}