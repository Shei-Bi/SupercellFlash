#pragma once

#include <stdint.h>
#include <filesystem>

#include "flash/flash_tags.h"
#include "flash/types/SWFStream.hpp"

// SWF Objects
#include "SWFTexture.h"
#include "ExportName.h"

#include "flash/transform/MatrixBank.h"

#include "flash/display_object/Shape.h"
#include "flash/display_object/MovieClip.h"
#include "flash/display_object/TextField.h"
#include "flash/display_object/MovieClipModifier.h"

#define MULTIRES_DEFAULT_SUFFIX "_highres"
#define LOWRES_DEFAULT_SUFFIX "_lowres"

namespace sc
{
	namespace flash {
		namespace SC2
		{
			class DataStorage;
		}

		class SupercellSWF
		{
		public:
			SupercellSWF() {};
			virtual ~SupercellSWF() = default;
			SupercellSWF(const SupercellSWF&) = default;
			SupercellSWF(SupercellSWF&&) = default;
			SupercellSWF& operator=(const SupercellSWF&) = default;
			SupercellSWF& operator=(SupercellSWF&&) = default;

		public:
			fs::path current_file;

			SWFVector<ExportName> exports;
			SWFVector<MatrixBank, uint32_t> matrixBanks;

			SWFVector<SWFTexture> textures;
			SWFVector<Shape> shapes;
			SWFVector<MovieClip> movieclips;
			SWFVector<TextField> textfields;
			SWFVector<MovieClipModifier> movieclip_modifiers;

		public:
			virtual void load(const std::filesystem::path& filePath);
			bool load_internal(const std::filesystem::path& filepath, bool is_texture);

			bool load_sc1(bool is_texture);
			void load_sc2(wk::Stream& stream);
			void load_sc2();

			virtual void save(const fs::path& filepath, Signature signature);
			void save_internal(bool is_texture, bool is_lowres);

			virtual void save_sc2(const fs::path& filepath) const;

			SWFStream stream;

		protected:
			bool load_tags();

			void save_tags();
			void save_textures(bool has_data, bool is_lowres);

		public:
			uint16_t GetDisplayObjectID(SWFString& name);
			DisplayObject& GetDisplayObjectByID(uint16_t id);
			MovieClip& GetDisplayObjectByName(SWFString& name);

		public:
			// Saves all textures to _tex.sc if true
			bool use_external_texture = false;
			bool use_multi_resolution = false;
			bool use_low_resolution = true;
			bool use_precision_matrix = false;

			// Saves all textures to zktx files if true and use_external_texture is true
			bool use_external_texture_files = false;

			// Saves custom properties in MovieClips
			bool save_custom_property = true;

			bool save_as_sc2 = false;

			bool low_memory_usage_mode = false;

			SWFString multi_resolution_suffix = MULTIRES_DEFAULT_SUFFIX;
			SWFString low_resolution_suffix = LOWRES_DEFAULT_SUFFIX;

		public:
			static bool IsSC2(wk::Stream& stream);
		};
	}
}
