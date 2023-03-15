#pragma once

#include "SupercellFlash/objects/DisplayObject.h"
#include "SupercellFlash/objects/MovieClipFrame.h"

namespace sc
{
	class SupercellSWF;

	struct MovieClipFrameElement
	{
		uint16_t instanceIndex;
		uint16_t matrixIndex;
		uint16_t colorTransformIndex;
	};

	struct DisplayObjectInstance
	{
		uint16_t id;
		uint8_t blend;
		std::string name;
	};

	struct ScalingGrid
	{
		float x;
		float y;
		float width;
		float height;
	};

	class MovieClip : public DisplayObject
	{
	public:
		MovieClip() { }
		virtual ~MovieClip() { }

		/*Vectors*/
	public:
		std::vector<MovieClipFrameElement> frameElements;
		std::vector<DisplayObjectInstance> instances;
		std::vector<MovieClipFrame> frames;

		/* Getters */
	public:
		uint8_t frameRate() { return m_frameRate; }
		ScalingGrid scalingGrid() { return *m_scalingGrid; }
		uint8_t matrixBankIndex() { return m_matrixBankIndex; }

		/* Setters */
	public:
		void frameRate(uint8_t rate) { m_frameRate = rate; }
		void scalingGrid(ScalingGrid* grid) { m_scalingGrid = grid; }
		void matrixBankIndex(uint8_t index) { m_matrixBankIndex = index; }

	public:
		void load(SupercellSWF* swf, uint8_t tag);
		void save(SupercellSWF* swf);

		bool isMovieClip() const override { return true; }

	private:
		uint8_t m_frameRate = 24;

		ScalingGrid* m_scalingGrid = nullptr;
		uint8_t m_matrixBankIndex = 0;
	};
}