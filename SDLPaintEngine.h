#ifndef QSDLPAINTENGINE_H
#define QSDLPAINTENGINE_H

#include <SDL.h>

#include <QPaintEngine>

class SDLPaintEngine : public QPaintEngine
{
	public:
		SDLPaintEngine(SDL_Surface *surface)
			: m_surface(surface)
		{
		}

		bool begin(QPaintDevice *pdev) override
		{
			Q_UNUSED(pdev);

			return true;
		}

		bool end() override
		{
			return true;
		}

		void updateState(const QPaintEngineState &state) override
		{
			Q_UNUSED(state);
		}

		void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override
		{
			const QImage &image = pm.toImage();

			auto surface = SDL_CreateRGBSurfaceFrom((void*)image.bits()
				, image.width()
				, image.height()
				, image.depth()
				, image.bytesPerLine()
				, 0x00FF0000
				, 0x0000FF00
				, 0x000000FF
				, 0xFF000000
				);

			SDL_Rect target = { r.x(), r.y(), r.width(), r.height() };

			SDL_BlitSurface(surface, nullptr, m_surface, &target);

			SDL_FreeSurface(surface);
		}

		QPoint coordinateOffset() const override
		{
			return QPoint();
		}

		Type type() const override
		{
			return QPaintEngine::User;
		}

	private:
		SDL_Surface *m_surface;
};

class SDLPaintDevice : public QPaintDevice
{
	public:
		SDLPaintDevice(SDL_Surface *surface)
			: m_engine(surface)
			, m_enginePtr(&m_engine)
		{
		}

		int metric(PaintDeviceMetric metric) const override
		{
			float ddpi;
			float hdpi;
			float vdpi;

			SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi);

			switch (metric)
			{
				case QPaintDevice::PdmWidth: return 800;
				case QPaintDevice::PdmHeight: return 600;
				case QPaintDevice::PdmDpiX: return hdpi;
				case QPaintDevice::PdmDpiY: return vdpi;

				default: return 0;
			}
		}

		QPaintEngine *paintEngine() const override
		{
			return m_enginePtr;
		}

	private:
		SDLPaintEngine m_engine;
		SDLPaintEngine *m_enginePtr;
};

#endif // QSDLPAINTENGINE_H
