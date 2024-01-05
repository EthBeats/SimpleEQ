/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider) {
	using namespace juce;
	
	auto bounds = Rectangle<float>(x, y, width, height);
	
	// fill sliders gray
	g.setColour(Colour(20u, 20u, 20u));
	g.fillEllipse(bounds);
	
	// draw borders white
	g.setColour(Colour(255u, 255u, 255u));
	g.drawEllipse(bounds, 1.f);
	
	if (auto *rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
		auto center = bounds.getCentre();
		Path p;
	
		Rectangle<float> r;
		r.setLeft(center.getX() - 2);
		r.setRight(center.getX() + 2);
		r.setTop(bounds.getY());
		r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
		
		p.addRoundedRectangle(r, 2.f);
		
		jassert(rotaryStartAngle < rotaryEndAngle);
	
		auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
	
		p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
	
		g.fillPath(p);
		
		g.setFont(rswl->getTextHeight());
		
		auto text = rswl->getDisplayString();
		auto strWidth = g.getCurrentFont().getStringWidth(text);
		
		r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
		r.setCentre(bounds.getCentre());
		
		g.setColour(Colours::black);
		g.fillRect(r);
		
		g.setColour(Colours::white);
		g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
	}
}

void LookAndFeel::drawToggleButton (juce::Graphics &g, juce::ToggleButton &toggleButton, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
	using namespace juce;
	
	if (auto *pb = dynamic_cast<PowerButton*>(&toggleButton)) {
		Path powerButton;
		
		auto bounds = toggleButton.getLocalBounds();
		auto size = jmin(bounds.getWidth(), bounds.getHeight() - 3);
		auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
		
		float ang = 35.f;
		size -= 7;
		
		powerButton.addCentredArc(r.getCentreX(), r.getCentreY(), size * 0.5f, size * 0.5f, 0.f, degreesToRadians(ang), degreesToRadians(360.f - ang), true);
		
		powerButton.startNewSubPath(r.getCentreX(), r.getY());
		powerButton.lineTo(r.getCentre());
		
		PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
		
		auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colours::orange;
		
		g.setColour(color);
		g.strokePath(powerButton, pst);
		g.drawEllipse(r, 2.f);
	}
	
	else if (auto *analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton)) {
		auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colours::orange;
		
		g.setColour(color);
		
		auto bounds = toggleButton.getLocalBounds();
		g.drawRect(bounds);
		
		g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
	}
}
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g) {
	using namespace juce;
	
	auto startAng = degreesToRadians(180.f + 45.f);
	auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
	
	auto range = getRange();
	
	auto sliderBounds = getSliderBounds();
	
//	g.setColour(Colours::red);
//	g.drawRect(getLocalBounds());
//	g.setColour(Colours::yellow);
//	g.drawRect(sliderBounds);

	
	getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0), startAng, endAng, *this);
	
	auto center = sliderBounds.toFloat().getCentre();
	auto radius = sliderBounds.getWidth() * 0.5f;
	
	// set labels color
	g.setColour(Colours::orange);
	g.setFont(getTextHeight());
	
	for (int i = 0; i < labels.size(); ++i) {
		auto pos = labels[i].pos;
		jassert(0.f <= pos);
		jassert(pos <= 1.f);
		
		auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
		
		auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
		
		Rectangle<float> r;
		auto str = labels[i].label;
		
		r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
		r.setCentre(c);
		r.setY(r.getY() + getTextHeight());
		
		g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
	}
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
	auto bounds = getLocalBounds();
	
	auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
	
	size -= getTextHeight() * 2;
	
	juce::Rectangle<int> r;
	r.setSize(size, size);
	r.setCentre(bounds.getCentreX(), 0);
	r.setY(2);
	
	return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const {
	if (auto *choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
		return choiceParam->getCurrentChoiceName();
	
	juce::String str;
	bool addK = false;
	
	if (auto *floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
		float val = getValue();
		
		if (val > 999.f) {
			val /= 1000.f;
			addK = true;
		}
		
		str = juce::String(val, (addK ? 2 : 0));
	} else {
		jassertfalse; // incorrect params!
	}
	
	if (suffix.isNotEmpty()) {
		str << " ";
		
		if (addK)
			str << "k";
			
		str << suffix;
	}
	
	return str;
}
//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor &p) : audioProcessor(p),
//	leftChannelFifo(&audioProcessor.leftChannelFifo)
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo) {
	const auto &params = audioProcessor.getParameters();
	
	for (auto param : params) {
		param->addListener(this);
	}
	
	/* basic math explanation...
	 * 44100 sample rate / 2048 = 21.53Hz per equal bin */
	
	updateChain();
	
	startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent() {
	const auto &params = audioProcessor.getParameters();
	
	for (auto param : params) {
		param->removeListener(this);
	}
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
	parametersChanged.set(true);
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate) {
	juce::AudioBuffer<float> tempIncomingBuffer;
	
	while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0) {
		if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer)) {
			auto size = tempIncomingBuffer.getNumSamples();
			
			// shift over data
			juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
											  monoBuffer.getReadPointer(0, size),
											  monoBuffer.getNumSamples() - size);
											  
			// copy to end (moving right to left with new data)
			juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
											  tempIncomingBuffer.getReadPointer(0, 0),
											  size);
											  
			leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
		}
	}
	
//	const auto fftBounds = getAnalysisArea().toFloat();
	const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
	
	/* bin width (44100 / 2048) -> sample rate / fftsize */
	const auto binWidth = sampleRate / (double)fftSize;
	
	/* if there are FFT data buffers to pull
		if we can pull them
			generate path */
	while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0) {
		std::vector<float> fftData;
		
		if (leftChannelFFTDataGenerator.getFFTData(fftData)) {
			pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
		}
	}
	
	while (pathProducer.getNumPathsAvailable()) {
		pathProducer.getPath(leftChannelFFTPath);
	}
}

void ResponseCurveComponent::timerCallback() {
	auto fftBounds = getAnalysisArea().toFloat();
	auto sampleRate = audioProcessor.getSampleRate();
	
	leftPathProducer.process(fftBounds, sampleRate);
	rightPathProducer.process(fftBounds, sampleRate);

	if (parametersChanged.compareAndSetBool(false, true)) {
		updateChain();
		//signal repaint
//		repaint();
	}
	
	repaint();
}

void ResponseCurveComponent::updateChain() {
	//update monochain
	auto chainSettings = getChainSettings(audioProcessor.apvts);
	
	monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
	monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
	monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
	
	auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
		
	auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
	auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
		
	updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
		
	updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
	updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
	using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    g.drawImage(background, getLocalBounds().toFloat());
    
	auto responseArea = getAnalysisArea();
    
    auto w = responseArea.getWidth();
    
    auto &lowcut = monoChain.get<ChainPositions::LowCut>();
    auto &peak = monoChain.get<ChainPositions::Peak>();
    auto &highcut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    
    mags.resize(w);
    
    for (int i = 0; i < w; ++i) {
		double mag = 1.f;
		auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
		
		if (! monoChain.isBypassed<ChainPositions::Peak>())
			mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
		
		if (! monoChain.isBypassed<ChainPositions::LowCut>()) {
			if (! lowcut.isBypassed<0>())
				mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (! lowcut.isBypassed<1>())
				mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (! lowcut.isBypassed<2>())
				mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (! lowcut.isBypassed<3>())
				mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
			
		if (! monoChain.isBypassed<ChainPositions::HighCut>()) {
			if (! highcut.isBypassed<0>())
				mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (! highcut.isBypassed<1>())
				mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (! highcut.isBypassed<2>())
				mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (! highcut.isBypassed<3>())
				mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
			
		mags[i] = Decibels::gainToDecibels(mag);
	}
	
	Path responseCurve;
	
	const double outputMin = responseArea.getBottom();
	const double outputMax = responseArea.getY();
	
	auto map = [outputMin, outputMax](double input) {
		return jmap(input, -24.0, 24.0, outputMin, outputMax);
	};
	
	responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
	
	for (size_t i = 1; i < mags.size(); ++i) {
		responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
	}
	
	auto leftChannelFFTPath = leftPathProducer.getPath();
	leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY() - 8));
	
	// set FFT left color
	g.setColour(Colours::skyblue);
	g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
	
	auto rightChannelFFTPath = rightPathProducer.getPath();
	rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY() - 8));
	
	// set FFT right color
	g.setColour(Colours::blue);
	g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
	
	// set border color
	g.setColour(Colours::orange);
	g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
	
	// set path color
	g.setColour(Colours::white);
	g.strokePath(responseCurve, PathStrokeType(2.f));
}

void ResponseCurveComponent::resized() {
	using namespace juce;
	
	background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
	Graphics g(background);
	
	// normal freq ranges
	Array<float> freqs {
		20, /*30, 40,*/ 50, 100,
		200, /*300, 400,*/ 500, 1000,
		2000, /*3000, 4000,*/ 5000, 10000,
		20000
	};
	
	auto renderArea = getAnalysisArea();
	auto left = renderArea.getX();
	auto right = renderArea.getRight();
	auto top = renderArea.getY();
	auto bottom = renderArea.getBottom();
	auto width = renderArea.getWidth();
	
	Array<float> xs;
	
	for (auto f : freqs) {
		auto normX = mapFromLog10(f, 20.f, 20000.f);
		xs.add(left + width * normX);
	}
	
	// set grid color
	g.setColour(Colour(100u, 100u, 100u));
	for (auto x : xs) {
		g.drawVerticalLine(x, top, bottom);
	}
	
	// normal gain ranges
	Array<float> gains {
		-24, -12, 0, 12, 24
	};
	
	for (auto gDb : gains) {
		auto normY = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
		
		g.setColour(gDb == 0.f ? Colours::orange : Colours::darkgrey);
		g.drawHorizontalLine(normY, left, right);
	}
	
	g.setColour(Colours::lightgrey);
	const int fontHeight = 10;
	g.setFont(fontHeight);
	
	// freq labels
	for (int i = 0; i < freqs.size(); ++i) {
		auto f = freqs[i];
		auto x = xs[i];
		
		bool addK = false;
		String str;
		
		if (f > 999.f) {
			addK = true;
			f /= 1000.f;
		}
		
		str << f;
		if (addK)
			str << "k";
		str << "Hz";
		
		auto textWidth = g.getCurrentFont().getStringWidth(str);
		
		Rectangle<int> r;
		r.setSize(textWidth, fontHeight);
		r.setCentre(x, 0);
		r.setY(1);
		
		g.drawFittedText(str, r, juce::Justification::centred, 1);
	}
	
	// gain labels
	for (auto gDb : gains) {
		auto normY = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
		
		String str;
		if (gDb > 0)
			str << "+";
		str << gDb;
		
		auto textWidth = g.getCurrentFont().getStringWidth(str);
		
		Rectangle<int> r;
		r.setSize(textWidth, fontHeight);
		r.setX(getWidth() - textWidth);
		r.setCentre(r.getCentreX(), normY);
		
		g.setColour(gDb == 0.f ? Colours::orange : Colours::lightgrey);
		
		g.drawFittedText(str, r, juce::Justification::centred, 1);
		
		// analyzer gains
		str.clear();
		str << (gDb - 24.f);
		
		textWidth = g.getCurrentFont().getStringWidth(str);
		r.setX(1);
		r.setSize(textWidth, fontHeight);
		g.setColour(Colours::lightgrey);
		
		g.drawFittedText(str, r, juce::Justification::centred, 1);
	}
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea() {
	auto bounds = getLocalBounds();

	bounds.removeFromTop(12);
	bounds.removeFromBottom(2);
	bounds.removeFromLeft(20);
	bounds.removeFromRight(20);
				  
	return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea() {
	auto bounds = getRenderArea();
	
	bounds.removeFromTop(4);
	bounds.removeFromBottom(4);
	
	return bounds;
}

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),
    
    responseCurveComponent(audioProcessor),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
    
    lowCutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowCutBypassButton),
    peakBypassButtonAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton),
    highCutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highCutBypassButton),
    analyzerEnabledButtonAttachment(audioProcessor.apvts, "Analyzer Enabled", analyzerEnabledButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    
    peakGainSlider.labels.add({0.f, "-24dB"});
    peakGainSlider.labels.add({1.f, "+24dB"});
    
    peakQualitySlider.labels.add({0.f, "0.1"});
    peakQualitySlider.labels.add({1.f, "10.0"});
    
    lowCutFreqSlider.labels.add({0.f, "20Hz"});
    lowCutFreqSlider.labels.add({1.f, "20kHz"});
    
    highCutFreqSlider.labels.add({0.f, "20Hz"});
    highCutFreqSlider.labels.add({1.f, "20kHz"});
    
    lowCutSlopeSlider.labels.add({0.f, "12"});
    lowCutSlopeSlider.labels.add({1.f, "48"});
    
    highCutSlopeSlider.labels.add({0.f, "12"});
    highCutSlopeSlider.labels.add({1.f, "48"});
    
    for (auto *comp : getComps()) {
		addAndMakeVisible(comp);
	}
	
	lowCutBypassButton.setLookAndFeel(&lnf);
	peakBypassButton.setLookAndFeel(&lnf);
	highCutBypassButton.setLookAndFeel(&lnf);
	analyzerEnabledButton.setLookAndFeel(&lnf);
    
    setSize (600, 480);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor() {
	lowCutBypassButton.setLookAndFeel(nullptr);
	peakBypassButton.setLookAndFeel(nullptr);
	highCutBypassButton.setLookAndFeel(nullptr);
	analyzerEnabledButton.setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
	using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    
    auto analyzerEnabledArea = bounds.removeFromTop(25);
    analyzerEnabledArea.setWidth(100);
    analyzerEnabledArea.setX(5);
    analyzerEnabledArea.removeFromTop(2);
    
    analyzerEnabledButton.setBounds(analyzerEnabledArea);
    
    bounds.removeFromTop(5);
    
    float hRatio = 25.f / 100.f; //JUCE_LIVE_CONSTANT(33) / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);
    
    responseCurveComponent.setBounds(responseArea);
    
    bounds.removeFromTop(5);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
	highCutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);
    
    peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps() {
	return {
		&peakFreqSlider, &peakGainSlider, &peakQualitySlider, &lowCutFreqSlider, &highCutFreqSlider, &lowCutSlopeSlider, &highCutSlopeSlider, &responseCurveComponent, &lowCutBypassButton, &peakBypassButton, &highCutBypassButton, &analyzerEnabledButton
	};
}
