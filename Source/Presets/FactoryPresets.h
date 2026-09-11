#pragma once
#include "../Parameters.h"
#include <initializer_list>
#include <string>
#include <vector>
namespace rv
{
struct Preset
{
    std::string id, name, category, tags;
    Params parameters;
    bool user = false;
};
using Values = std::initializer_list<std::pair<P, float>>;
inline void apply(Params &p, Values values)
{
    for (auto [id, value] : values)
        p[id] = value;
}
inline std::vector<Preset> factoryPresets()
{
    std::vector<Preset> result;
    result.reserve(100);
    Params base;
    auto category = [&](Values v)
    {
        base = Params{};
        apply(base, v);
    };
    std::string group, tags;
    auto add = [&](const char *name, Values v)
    {
        Preset p;
        p.id = "factory-" + std::to_string(result.size() + 1);
        p.name = name;
        p.category = group;
        p.tags = tags;
        p.parameters = base;
        apply(p.parameters, v);
        p.parameters.sanitize();
        result.push_back(std::move(p));
    };
    group = "ROBOT";
    tags = "robot classic vocal";
    category({{P::wave, 1}, {P::bands, 2}, {P::unison, 1}, {P::character, .4f}});
    add("Classic Robot", {{P::envRelease, 85},
                          {P::tilt, 2},
                          {P::clarity, .65f},
                          {P::sibilance, .35f},
                          {P::warmth, .25f},
                          {P::width, .8f}});
    add("Chrome Robot", {{P::bands, 4},
                         {P::wave, 5},
                         {P::unison, 3},
                         {P::detune, 7},
                         {P::presence, 4},
                         {P::modern, .8f},
                         {P::exciter, .15f}});
    add("Soft Robot", {{P::wave, 2},
                       {P::oscMix, .25f},
                       {P::envAttack, 12},
                       {P::envRelease, 140},
                       {P::warmth, .5f},
                       {P::air, -3},
                       {P::clarity, .3f}});
    add("Giant Robot", {{P::rootNote, 36},
                        {P::size, .85f},
                        {P::throat, .7f},
                        {P::body, 4},
                        {P::drive, .4f},
                        {P::freqMin, 50},
                        {P::width, 1.3f}});
    add("Tiny Robot", {{P::rootNote, 65},
                       {P::formant, 7},
                       {P::size, -.5f},
                       {P::nasal, .45f},
                       {P::low, -6},
                       {P::envRelease, 40},
                       {P::bands, 1}});
    add("Dark Robot", {{P::wave, 0},
                       {P::rootNote, 41},
                       {P::formant, -4},
                       {P::tilt, -5},
                       {P::vintage, .6f},
                       {P::drive, .35f},
                       {P::air, -5}});
    add("Wide Robot", {{P::wave, 5},
                       {P::unison, 7},
                       {P::detune, 16},
                       {P::width, 1.6f},
                       {P::spread, .6f},
                       {P::unisonWidth, 1},
                       {P::bands, 3}});
    add("Broken Robot", {{P::crusher, .55f},
                         {P::bitDepth, 6},
                         {P::reduction, 7},
                         {P::bandShift, 2},
                         {P::motion, .45f},
                         {P::lfoShape, 2},
                         {P::formantMotion, .8f}});
    add("Servo Talk", {{P::bands, 0},
                       {P::wave, 4},
                       {P::envRelease, 22},
                       {P::definition, .9f},
                       {P::speaker, .3f},
                       {P::drive, .3f},
                       {P::nasal, .3f}});
    add("Silver Diplomat", {{P::bands, 5},
                            {P::wave, 0},
                            {P::formant, 1},
                            {P::modMix, .08f},
                            {P::clarity, .8f},
                            {P::modern, .9f},
                            {P::body, 2}});
    group = "FRENCH / ELECTRO";
    tags = "electro disco funk warm";
    category(
        {{P::wave, 5}, {P::bands, 2}, {P::detune, 8}, {P::warmth, .5f}, {P::drive, .25f}, {P::clarity, .6f}});
    add("French Chrome", {{P::presence, 3},
                          {P::envAttack, 2},
                          {P::envRelease, 65},
                          {P::width, 1.25f},
                          {P::formant, 1},
                          {P::sibilance, .4f}});
    add("Digital Romance", {{P::chord, 3},
                            {P::rootNote, 45},
                            {P::envRelease, 145},
                            {P::detune, 14},
                            {P::breath, .25f},
                            {P::motion, .1f},
                            {P::air, 3}});
    add("Disco Android", {{P::wave, 1},
                          {P::chord, 1},
                          {P::envRelease, 48},
                          {P::bands, 1},
                          {P::tilt, 3},
                          {P::drive, .4f},
                          {P::body, 3}});
    add("Funk Machine", {{P::wave, 4},
                         {P::rootNote, 43},
                         {P::envAttack, 1},
                         {P::envRelease, 30},
                         {P::definition, .9f},
                         {P::nasal, .3f},
                         {P::speaker, .15f}});
    add("Electric Human", {{P::bands, 4},
                           {P::modMix, .16f},
                           {P::formant, -1},
                           {P::modern, .7f},
                           {P::clarity, .85f},
                           {P::width, 1.1f},
                           {P::drive, .15f}});
    add("Future Disco", {{P::chord, 3},
                         {P::rootNote, 48},
                         {P::motion, .4f},
                         {P::tempoSync, 1},
                         {P::division, 3},
                         {P::autoPan, .4f},
                         {P::spread, .4f}});
    add("Analog Love", {{P::wave, 0},
                        {P::oscMix, .3f},
                        {P::vintage, .55f},
                        {P::formant, -2},
                        {P::envRelease, 170},
                        {P::air, -2},
                        {P::warmth, .7f}});
    add("Neon Vocal", {{P::bands, 5},
                       {P::exciter, .25f},
                       {P::detune, 18},
                       {P::width, 1.5f},
                       {P::air, 5},
                       {P::identity, .3f},
                       {P::envRelease, 55}});
    add("Midnight Boulevard", {{P::rootNote, 40},
                               {P::chord, 2},
                               {P::size, .3f},
                               {P::vintage, .4f},
                               {P::motion, .25f},
                               {P::lfoRate, .12f},
                               {P::filterMotion, .6f}});
    add("Velvet Circuit", {{P::wave, 2},
                           {P::chord, 3},
                           {P::rootNote, 52},
                           {P::body, 4},
                           {P::warmth, .8f},
                           {P::breath, .3f},
                           {P::width, 1.4f}});
    group = "VINTAGE";
    tags = "vintage analog warm narrow";
    category(
        {{P::vintage, .65f}, {P::bands, 1}, {P::wave, 0}, {P::unison, 1}, {P::width, .7f}, {P::modern, 0}});
    add("Seventies Voice", {{P::freqMax, 6500},
                            {P::envRelease, 110},
                            {P::warmth, .65f},
                            {P::tilt, -2},
                            {P::formantQ, .3f},
                            {P::sibilance, .4f}});
    add("Eighties Voltage", {{P::wave, 1},
                             {P::bands, 2},
                             {P::detune, 8},
                             {P::unison, 2},
                             {P::presence, 3},
                             {P::width, 1.2f},
                             {P::vintage, .4f}});
    add("Tape Robot", {{P::warmth, .8f},
                       {P::noise, .18f},
                       {P::motion, .12f},
                       {P::lfoRate, .3f},
                       {P::formantMotion, .25f},
                       {P::air, -4},
                       {P::drive, .2f}});
    add("Analog Radio", {{P::speaker, .7f},
                         {P::freqMin, 280},
                         {P::freqMax, 4800},
                         {P::nasal, .3f},
                         {P::width, 0},
                         {P::noise, .22f},
                         {P::drive, .4f}});
    add("Vintage Choir", {{P::chord, 3},
                          {P::rootNote, 43},
                          {P::wave, 5},
                          {P::unison, 5},
                          {P::envRelease, 220},
                          {P::width, 1.35f},
                          {P::vintage, .5f}});
    add("Old Computer", {{P::bands, 0},
                         {P::wave, 4},
                         {P::crusher, .4f},
                         {P::bitDepth, 8},
                         {P::reduction, 3},
                         {P::envRelease, 28},
                         {P::breath, 0}});
    add("Copper Ensemble", {{P::chord, 1},
                            {P::rootNote, 40},
                            {P::body, 5},
                            {P::formant, -3},
                            {P::warmth, .5f},
                            {P::spread, .3f},
                            {P::envRelease, 160}});
    add("Archive 1978", {{P::bands, 0},
                         {P::freqMax, 5000},
                         {P::drive, .45f},
                         {P::speaker, .35f},
                         {P::noise, .3f},
                         {P::clarity, .35f},
                         {P::air, -6}});
    add("Magnetic Velvet", {{P::wave, 2},
                            {P::oscMix, .45f},
                            {P::envAttack, 18},
                            {P::envRelease, 240},
                            {P::warmth, .9f},
                            {P::breath, .3f},
                            {P::width, 1}});
    add("Retro Broadcast", {{P::bands, 3},
                            {P::modMix, .12f},
                            {P::speaker, .45f},
                            {P::presence, 5},
                            {P::clarity, .8f},
                            {P::drive, .3f},
                            {P::width, .4f}});
    group = "MODERN";
    tags = "clean pop bright definition";
    category({{P::bands, 5},
              {P::modern, .9f},
              {P::clarity, .8f},
              {P::definition, .7f},
              {P::drive, .05f},
              {P::warmth, 0}});
    add("Hyper Clean", {{P::wave, 0},
                        {P::unison, 1},
                        {P::freqMax, 15000},
                        {P::envRelease, 55},
                        {P::sibilance, .5f},
                        {P::character, .1f}});
    add("Wide Modern", {{P::wave, 5},
                        {P::unison, 7},
                        {P::detune, 11},
                        {P::width, 1.7f},
                        {P::spread, .5f},
                        {P::air, 3},
                        {P::envRelease, 90}});
    add("Pop Robot", {{P::bands, 3},
                      {P::modMix, .12f},
                      {P::body, 2},
                      {P::presence, 3},
                      {P::formant, 1},
                      {P::sibilance, .45f},
                      {P::width, 1.25f}});
    add("Future Voice", {{P::formant, 3},
                         {P::identity, .2f},
                         {P::exciter, .2f},
                         {P::motion, .22f},
                         {P::lfoRate, .2f},
                         {P::widthMotion, .6f},
                         {P::air, 4}});
    add("Glass Vocal", {{P::wave, 2},
                        {P::oscMix, .6f},
                        {P::rootNote, 55},
                        {P::bands, 6},
                        {P::envAttack, 8},
                        {P::air, 6},
                        {P::breath, .4f}});
    add("Ultra Definition", {{P::bands, 6},
                             {P::envAttack, .6f},
                             {P::envRelease, 30},
                             {P::definition, 1},
                             {P::clarity, 1},
                             {P::unvoiced, .65f},
                             {P::modMix, .05f}});
    add("Prism Lead", {{P::wave, 4},
                       {P::rootNote, 60},
                       {P::formant, 2},
                       {P::spread, .7f},
                       {P::presence, 4},
                       {P::envRelease, 48},
                       {P::drive, .2f}});
    add("Studio Android", {{P::bands, 4},
                           {P::formant, -1},
                           {P::body, 2},
                           {P::modMix, .18f},
                           {P::width, .95f},
                           {P::envRelease, 80},
                           {P::sibilance, .55f}});
    add("Crystal Motion", {{P::chord, 1},
                           {P::motion, .5f},
                           {P::tempoSync, 1},
                           {P::division, 2},
                           {P::rhythm, 1},
                           {P::autoPan, .55f},
                           {P::formantMotion, .5f}});
    add("White Titanium", {{P::wave, 1},
                           {P::unison, 2},
                           {P::detune, 5},
                           {P::bands, 6},
                           {P::formantQ, .7f},
                           {P::tilt, 5},
                           {P::exciter, .3f}});
    group = "DARK";
    tags = "dark aggressive industrial";
    category({{P::rootNote, 36},
              {P::formant, -4},
              {P::tilt, -3},
              {P::drive, .5f},
              {P::vintage, .35f},
              {P::air, -3}});
    add("Demon Machine", {{P::size, .6f},
                          {P::throat, .8f},
                          {P::distortion, .4f},
                          {P::bands, 2},
                          {P::body, 5},
                          {P::width, 1.4f}});
    add("Industrial", {{P::wave, 4},
                       {P::bands, 0},
                       {P::distortion, .65f},
                       {P::crusher, .25f},
                       {P::bitDepth, 7},
                       {P::envRelease, 45},
                       {P::speaker, .25f}});
    add("Dark Transmission", {{P::speaker, .6f},
                              {P::noise, .3f},
                              {P::freqMax, 5200},
                              {P::bandShift, -1},
                              {P::width, .5f},
                              {P::envRelease, 160},
                              {P::breath, .4f}});
    add("Mechanical Breath", {{P::wave, 6},
                              {P::breath, .7f},
                              {P::unvoiced, .8f},
                              {P::bands, 5},
                              {P::envRelease, 130},
                              {P::formant, -6},
                              {P::width, 1.3f}});
    add("Broken Speaker", {{P::speaker, 1},
                           {P::distortion, .8f},
                           {P::crusher, .5f},
                           {P::bitDepth, 5},
                           {P::reduction, 5},
                           {P::width, 0},
                           {P::nasal, .7f}});
    add("Cyber Villain", {{P::wave, 1},
                          {P::chord, 4},
                          {P::size, .5f},
                          {P::formantQ, .75f},
                          {P::presence, 4},
                          {P::drive, .7f},
                          {P::clarity, .7f}});
    add("Obsidian Choir", {{P::chord, 2},
                           {P::unison, 7},
                           {P::detune, 22},
                           {P::envRelease, 270},
                           {P::spread, .65f},
                           {P::width, 1.7f},
                           {P::warmth, .7f}});
    add("Reactor Core", {{P::bands, 1},
                         {P::wave, 0},
                         {P::motion, .75f},
                         {P::lfoRate, 4},
                         {P::formantMotion, .65f},
                         {P::bandMotion, .5f},
                         {P::distortion, .3f}});
    add("Night Sentinel", {{P::bands, 6},
                           {P::modMix, .07f},
                           {P::clarity, .9f},
                           {P::gateThreshold, -40},
                           {P::throat, .4f},
                           {P::speaker, .3f},
                           {P::body, 3}});
    add("Black Static", {{P::wave, 6},
                         {P::crusher, .7f},
                         {P::bitDepth, 4},
                         {P::reduction, 11},
                         {P::noise, .6f},
                         {P::motion, .4f},
                         {P::lfoShape, 2}});
    group = "SOFT";
    tags = "soft ambient airy dream";
    category({{P::drive, 0},
              {P::character, .1f},
              {P::envAttack, 14},
              {P::envRelease, 230},
              {P::wave, 2},
              {P::breath, .35f},
              {P::clarity, .35f}});
    add("Dream Vocoder", {{P::chord, 3},
                          {P::rootNote, 48},
                          {P::width, 1.4f},
                          {P::air, 4},
                          {P::warmth, .35f},
                          {P::motion, .12f}});
    add("Airy", {{P::wave, 6},
                 {P::bands, 5},
                 {P::air, 7},
                 {P::breath, .6f},
                 {P::unvoiced, .65f},
                 {P::low, -4},
                 {P::envRelease, 160}});
    add("Whisper Robot", {{P::wave, 6},
                          {P::formant, -2},
                          {P::sibilance, .6f},
                          {P::breath, .8f},
                          {P::envAttack, 5},
                          {P::speaker, .1f},
                          {P::width, .8f}});
    add("Ambient Signal", {{P::chord, 2},
                           {P::rootNote, 43},
                           {P::envRelease, 450},
                           {P::unison, 5},
                           {P::detune, 20},
                           {P::motion, .35f},
                           {P::lfoRate, .06f}});
    add("Soft Synth", {{P::wave, 3},
                       {P::oscMix, .8f},
                       {P::bands, 1},
                       {P::rootNote, 55},
                       {P::modMix, .2f},
                       {P::warmth, .6f},
                       {P::body, 4}});
    add("Floating Voice", {{P::wave, 5},
                           {P::chord, 1},
                           {P::unison, 7},
                           {P::detune, 25},
                           {P::width, 1.8f},
                           {P::spread, .5f},
                           {P::formant, 2}});
    add("Pearl Haze", {{P::wave, 2},
                       {P::formant, 4},
                       {P::envRelease, 300},
                       {P::air, 5},
                       {P::motion, .25f},
                       {P::autoPan, .5f},
                       {P::lfoRate, .11f}});
    add("Lunar Lullaby", {{P::rootNote, 40},
                          {P::size, .4f},
                          {P::chord, 2},
                          {P::warmth, .7f},
                          {P::vintage, .3f},
                          {P::body, 3},
                          {P::air, 1}});
    add("Silk Current", {{P::bands, 6},
                         {P::modMix, .14f},
                         {P::clarity, .65f},
                         {P::envAttack, 8},
                         {P::envRelease, 180},
                         {P::width, 1.15f},
                         {P::presence, 3}});
    add("Cloud Passenger", {{P::wave, 5},
                            {P::chord, 3},
                            {P::rootNote, 55},
                            {P::detune, 30},
                            {P::envRelease, 400},
                            {P::motion, .4f},
                            {P::widthMotion, .8f}});
    group = "WEIRD";
    tags = "weird alien glitch experimental";
    category({{P::bands, 1}, {P::character, .65f}, {P::motion, .4f}, {P::formantMotion, .5f}});
    add("Alien", {{P::formant, 8},
                  {P::wave, 4},
                  {P::nasal, .65f},
                  {P::bandShift, -2},
                  {P::lfoRate, .7f},
                  {P::rootNote, 57}});
    add("Tiny Creature", {{P::rootNote, 72},
                          {P::formant, 10},
                          {P::size, -.9f},
                          {P::envRelease, 30},
                          {P::speaker, .35f},
                          {P::low, -10},
                          {P::motion, .15f}});
    add("Giant Creature", {{P::rootNote, 27},
                           {P::size, 1},
                           {P::throat, 1},
                           {P::formant, -8},
                           {P::bands, 0},
                           {P::drive, .5f},
                           {P::body, 6}});
    add("Radio Ghost", {{P::wave, 6},
                        {P::speaker, .8f},
                        {P::breath, .7f},
                        {P::noise, .4f},
                        {P::envRelease, 320},
                        {P::lfoRate, .08f},
                        {P::autoPan, .7f}});
    add("Digital Corruption", {{P::crusher, .9f},
                               {P::bitDepth, 4},
                               {P::reduction, 18},
                               {P::distortion, .3f},
                               {P::lfoShape, 2},
                               {P::bandMotion, 1},
                               {P::lfoRate, 6}});
    add("Glitch Voice", {{P::envRelease, 10},
                         {P::bands, 0},
                         {P::tempoSync, 1},
                         {P::division, 4},
                         {P::lfoShape, 2},
                         {P::motion, 1},
                         {P::formantMotion, 1}});
    add("Bit Machine", {{P::wave, 1},
                        {P::crusher, 1},
                        {P::bitDepth, 5},
                        {P::reduction, 6},
                        {P::bands, 2},
                        {P::formantQ, .8f},
                        {P::motion, .1f}});
    add("Telephone Intelligence", {{P::speaker, 1},
                                   {P::freqMin, 350},
                                   {P::freqMax, 3500},
                                   {P::modMix, .12f},
                                   {P::width, 0},
                                   {P::nasal, .7f},
                                   {P::motion, .05f}});
    add("Broken Android", {{P::wave, 4},
                           {P::bandShift, 4},
                           {P::crusher, .5f},
                           {P::reduction, 9},
                           {P::formant, -6},
                           {P::lfoRate, 2.5f},
                           {P::stereoMod, .8f}});
    add("Liquid Geometry", {{P::bands, 5},
                            {P::formantQ, .85f},
                            {P::motion, .85f},
                            {P::lfoRate, .18f},
                            {P::filterMotion, 1},
                            {P::bandMotion, .9f},
                            {P::spread, .8f}});
    group = "CHOIR";
    tags = "choir chords wide polyphonic";
    category({{P::chord, 3},
              {P::wave, 5},
              {P::unison, 5},
              {P::detune, 16},
              {P::envRelease, 190},
              {P::spread, .4f},
              {P::width, 1.4f}});
    add("Robot Choir", {{P::bands, 2},
                        {P::rootNote, 48},
                        {P::formant, 1},
                        {P::warmth, .4f},
                        {P::clarity, .6f},
                        {P::presence, 2}});
    add("Wide Choir", {{P::unison, 7},
                       {P::detune, 23},
                       {P::width, 1.9f},
                       {P::unisonWidth, 1},
                       {P::spread, .7f},
                       {P::bands, 4},
                       {P::air, 4}});
    add("Synthetic Choir", {{P::wave, 1},
                            {P::chord, 1},
                            {P::rootNote, 43},
                            {P::formantQ, .7f},
                            {P::bands, 3},
                            {P::exciter, .25f},
                            {P::envRelease, 130}});
    add("Dark Choir", {{P::chord, 2},
                       {P::rootNote, 36},
                       {P::formant, -4},
                       {P::size, .5f},
                       {P::tilt, -4},
                       {P::vintage, .6f},
                       {P::warmth, .6f}});
    add("Air Choir", {{P::wave, 6},
                      {P::bands, 6},
                      {P::air, 7},
                      {P::breath, .6f},
                      {P::unvoiced, .7f},
                      {P::envRelease, 250},
                      {P::drive, 0}});
    add("Cathedral Circuit", {{P::chord, 4},
                              {P::rootNote, 40},
                              {P::envAttack, 30},
                              {P::envRelease, 480},
                              {P::motion, .3f},
                              {P::lfoRate, .05f},
                              {P::formantMotion, .35f}});
    add("Neon Ensemble", {{P::rootNote, 55},
                          {P::chord, 3},
                          {P::bands, 5},
                          {P::modern, 1},
                          {P::exciter, .2f},
                          {P::air, 5},
                          {P::envRelease, 120}});
    add("Minor Satellites", {{P::chord, 2},
                             {P::rootNote, 45},
                             {P::motion, .55f},
                             {P::tempoSync, 1},
                             {P::division, 1},
                             {P::autoPan, .6f},
                             {P::widthMotion, .7f}});
    add("Octave Assembly", {{P::chord, 4},
                            {P::rootNote, 48},
                            {P::wave, 4},
                            {P::detune, 6},
                            {P::bands, 1},
                            {P::drive, .4f},
                            {P::definition, .7f}});
    add("Velour Voices", {{P::wave, 2},
                          {P::rootNote, 43},
                          {P::oscMix, .4f},
                          {P::warmth, .8f},
                          {P::envRelease, 300},
                          {P::body, 4},
                          {P::breath, .3f}});
    group = "BASS VOICE";
    tags = "bass deep sub heavy";
    category({{P::rootNote, 31},
              {P::freqMin, 40},
              {P::formant, -5},
              {P::size, .6f},
              {P::low, 3},
              {P::body, 3},
              {P::width, .65f}});
    add("Deep Machine", {{P::wave, 0},
                         {P::bands, 2},
                         {P::drive, .45f},
                         {P::throat, .6f},
                         {P::clarity, .7f},
                         {P::envRelease, 85}});
    add("Sub Robot", {{P::wave, 1},
                      {P::oscMix, .5f},
                      {P::rootNote, 28},
                      {P::bands, 0},
                      {P::low, 6},
                      {P::warmth, .7f},
                      {P::air, -5}});
    add("Giant Voice", {{P::chord, 4},
                        {P::rootNote, 36},
                        {P::size, 1},
                        {P::formant, -8},
                        {P::throat, .8f},
                        {P::width, 1.2f},
                        {P::drive, .5f}});
    add("Bass Transmission", {{P::speaker, .6f},
                              {P::noise, .18f},
                              {P::formant, -3},
                              {P::presence, 5},
                              {P::distortion, .2f},
                              {P::bands, 1},
                              {P::envRelease, 110}});
    add("Rubber Voltage", {{P::wave, 4},
                           {P::rootNote, 34},
                           {P::nasal, .6f},
                           {P::motion, .5f},
                           {P::lfoRate, 1.5f},
                           {P::filterMotion, .8f},
                           {P::formantMotion, .65f}});
    add("Subway Oracle", {{P::wave, 0},
                          {P::rootNote, 29},
                          {P::bands, 4},
                          {P::modMix, .12f},
                          {P::clarity, .8f},
                          {P::vintage, .4f},
                          {P::throat, 1}});
    add("Carbon Bass", {{P::wave, 1},
                        {P::distortion, .55f},
                        {P::crusher, .2f},
                        {P::bitDepth, 8},
                        {P::bands, 1},
                        {P::width, .9f},
                        {P::envRelease, 45}});
    add("Low Orbit", {{P::chord, 1},
                      {P::rootNote, 33},
                      {P::wave, 5},
                      {P::detune, 20},
                      {P::width, 1.5f},
                      {P::envRelease, 260},
                      {P::air, 2}});
    add("Baritone Chrome", {{P::rootNote, 40},
                            {P::formant, -2},
                            {P::size, .3f},
                            {P::bands, 5},
                            {P::modern, .9f},
                            {P::clarity, .9f},
                            {P::presence, 4}});
    add("Foundation Pulse", {{P::wave, 4},
                             {P::rootNote, 26},
                             {P::envAttack, 1},
                             {P::envRelease, 35},
                             {P::definition, .9f},
                             {P::drive, .6f},
                             {P::width, 0}});
    group = "SPECIAL FX";
    tags = "fx speaker radio transmission";
    category({{P::wave, 0}, {P::bands, 1}, {P::speaker, .75f}, {P::width, .4f}, {P::drive, .35f}});
    add("Intercom", {{P::freqMin, 300},
                     {P::freqMax, 5000},
                     {P::modMix, .2f},
                     {P::presence, 4},
                     {P::noise, .1f},
                     {P::envRelease, 65}});
    add("Megaphone", {{P::speaker, 1},
                      {P::distortion, .4f},
                      {P::nasal, .6f},
                      {P::modMix, .3f},
                      {P::bands, 2},
                      {P::body, -4},
                      {P::width, 0}});
    add("Radio", {{P::vintage, .75f},
                  {P::noise, .3f},
                  {P::freqMax, 5500},
                  {P::warmth, .6f},
                  {P::modMix, .1f},
                  {P::air, -6},
                  {P::envRelease, 100}});
    add("Telephone", {{P::speaker, 1},
                      {P::freqMin, 380},
                      {P::freqMax, 3400},
                      {P::width, 0},
                      {P::modMix, .4f},
                      {P::nasal, .3f},
                      {P::drive, .15f}});
    add("Pocket Speaker", {{P::speaker, 1},
                           {P::body, -6},
                           {P::mid, 4},
                           {P::distortion, .2f},
                           {P::bands, 3},
                           {P::rootNote, 55},
                           {P::width, .2f}});
    add("Transmission", {{P::crusher, .35f},
                         {P::bitDepth, 9},
                         {P::reduction, 4},
                         {P::noise, .4f},
                         {P::envRelease, 40},
                         {P::gateThreshold, -38},
                         {P::clarity, .8f}});
    add("Space Communication", {{P::wave, 4},
                                {P::formant, 4},
                                {P::motion, .3f},
                                {P::lfoRate, .22f},
                                {P::autoPan, .7f},
                                {P::noise, .25f},
                                {P::envRelease, 200}});
    add("Emergency Channel", {{P::wave, 1},
                              {P::rootNote, 62},
                              {P::speaker, .9f},
                              {P::distortion, .45f},
                              {P::envRelease, 18},
                              {P::definition, 1},
                              {P::presence, 6}});
    add("Underwater Terminal", {{P::formant, -7},
                                {P::freqMax, 2500},
                                {P::vintage, 1},
                                {P::envRelease, 300},
                                {P::motion, .6f},
                                {P::lfoRate, .4f},
                                {P::filterMotion, 1}});
    add("Last Signal", {{P::wave, 6},
                        {P::crusher, .6f},
                        {P::bitDepth, 6},
                        {P::reduction, 14},
                        {P::noise, .5f},
                        {P::breath, .6f},
                        {P::motion, .55f},
                        {P::lfoShape, 2}});
    // Fixed trims measured from the bundled local speech test, never an online AGC.
    const std::array<float, 100> trims{
        -4.3f, 11.7f, -2.3f,  -7.3f, -8.2f, -4.7f, 11.3f, 1.0f,  -5.0f, 6.8f,  4.9f,  -3.2f, -10.1f,
        2.2f,  4.0f,  -1.9f,  8.7f,  9.1f,  -1.4f, -9.5f, -5.0f, -0.5f, -6.1f, 7.0f,  1.2f,  -5.8f,
        -7.2f, -2.5f, -10.7f, 4.5f,  12.5f, 16.7f, 7.0f,  14.8f, 6.5f,  12.6f, 7.3f,  6.8f,  11.3f,
        8.2f,  -3.1f, -9.1f,  11.4f, 10.7f, 7.9f,  -4.9f, -2.7f, -0.8f, 5.7f,  21.8f, -2.4f, 17.1f,
        16.8f, 9.7f,  -2.8f,  10.4f, 8.6f,  2.7f,  7.9f,  10.9f, 0.6f,  4.1f,  -3.4f, 14.0f, 6.8f,
        6.2f,  0.1f,  11.5f,  6.1f,  11.7f, 0.3f,  1.6f,  2.5f,  2.8f,  13.0f, 4.1f,  9.5f,  3.4f,
        -4.1f, -2.1f, 3.4f,   -8.0f, -1.7f, 11.2f, 4.6f,  7.2f,  -5.6f, 0.7f,  13.1f, -0.2f, 7.4f,
        2.9f,  7.5f,  3.8f,   10.2f, 9.8f,  5.0f,  1.9f,  12.5f, 16.7f};
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i].parameters[P::presetLevel] = trims[i];
    return result;
}
} // namespace rv
