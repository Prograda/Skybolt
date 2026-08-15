// Copyright 2026 Ozan İrsoy
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BARKEEP_H
#define BARKEEP_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <variant>
#include <vector>

#define BARKEEP_VERSION "0.1.6"

#if defined(BARKEEP_ENABLE_FMT_FORMAT)
#include <fmt/format.h>
#include <fmt/ostream.h>
#elif defined(BARKEEP_ENABLE_STD_FORMAT)
#include <format>
#endif

namespace barkeep {

using Strings = std::vector<std::string>;
using StringsList = std::vector<Strings>;

// double precision seconds
using Duration = std::chrono::duration<double, std::ratio<1>>;

/// Kind of animation being displayed for Animation.
enum AnimationStyle : unsigned short {
  Ellipsis,
  Clock,
  Moon,
  Earth,
  Bar,
  UnicodeBar,
  Bounce,
};

/// Definitions of various stills for Animation. AnimationStyle indexes into
/// this.
const static std::vector<std::pair<Strings, double>> animation_stills_{
    {{".  ", ".. ", "..."}, 0.5},
    {{"🕐", "🕜", "🕑", "🕝", "🕒", "🕞", "🕓", "🕟", "🕔", "🕠", "🕕", "🕡",
      "🕖", "🕢", "🕗", "🕣", "🕘", "🕤", "🕙", "🕥", "🕚", "🕦", "🕛", "🕧"},
     0.5},
    {{"🌕", "🌖", "🌗", "🌘", "🌑", "🌒", "🌓", "🌔"}, 0.5},
    {{"🌎", "🌍", "🌏"}, 0.5},
    {{"-", "/", "|", "\\"}, 0.5},
    {{"╶─╴", " ╱ ", " │ ", " ╲ "}, 0.5},
    {{
         "●                  ", "●                  ", "●                  ",
         "●                  ", " ●                 ", "  ●                ",
         "   ●               ", "     ●             ", "       ●           ",
         "         ●         ", "           ●       ", "             ●     ",
         "               ●   ", "                ●  ", "                 ● ",
         "                  ●", "                  ●", "                  ●",
         "                  ●", "                 ● ", "                ●  ",
         "               ●   ", "             ●     ", "           ●       ",
         "         ●         ", "       ●           ", "     ●             ",
         "   ●               ", "  ●                ", " ●                 ",
     },
     0.05}};

struct BarParts {
  std::string left;
  std::string right;
  Strings fill;
  Strings empty;

  // below are optionally used for coloring
  std::string incomplete_left_modifier = "";
  std::string complete_left_modifier = "";
  std::string middle_modifier = "";
  std::string right_modifier = "";

  std::string percent_left_modifier = "";
  std::string percent_right_modifier = "";

  std::string value_left_modifier = "";
  std::string value_right_modifier = "";

  std::string speed_left_modifier = "";
  std::string speed_right_modifier = "";
};

const static auto red = "\033[31m";
const static auto green = "\033[32m";
const static auto yellow = "\033[33m";
const static auto blue = "\033[34m";
const static auto magenta = "\033[35m";
const static auto cyan = "\033[36m";
const static auto reset = "\033[0m";

/// Kind of bar being displayed for ProgressBar.
enum ProgressBarStyle : unsigned short {
  Bars,   ///< Simple ascii pipes
  Blocks, ///< Unicode blocks with fine granularity fill
  Rich,   ///< A style inspired by Rich library, also used by pip
  Line,   ///< Minimalist unicode line inspired by Rich style
};

/// Definitions of various partial bars for ProgressBar.
/// ProgressBarStyle indexes into this.
const static std::vector<BarParts> progress_bar_parts_{
    {"|", "|", {"|"}, {" "}},
    {"|", "|", {"▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"}, {" "}},
    {"",
     "",
     {"╸", "━"},
     {"╺", "━"},
     "\033[38;2;249;38;114m",
     "\033[38;2;114;156;31m",
     "\033[38;5;237m",
     reset,
     cyan,
     reset,
     green,
     reset,
     red,
     reset},
    {"", "", {"╾", "━"}, {"─"}},
};

class BaseDisplay;

/// Class to handle running display loop within a worker thread.
class AsyncDisplayer {
 protected:
  std::ostream* out_ = &std::cout; ///< output stream to write
  BaseDisplay* parent_ = nullptr;  ///< parent display to render which owns this

  // state
  std::unique_ptr<std::thread> displayer_thread_;
  std::condition_variable done_cv_;
  std::mutex done_m_;
  std::atomic<bool> done_ = false; ///< if true, display can be stopped
  long last_num_newlines_ = 0;

  // configuration
  Duration interval_;   ///< interval to refresh and check if complete
  bool no_tty_ = false; ///< true if output is not a tty

  /// Display everything (message, maybe with animation, progress bar, etc).
  /// @param redraw If true, display is assumed to be redrawn. This, e.g. means
  ///               an Animation should not increment the still frame index.
  void display_(bool redraw = false);

  /// Join the display thread. Protected because python bindings need to
  /// override to handle GIL.
  virtual void join() {
    displayer_thread_->join();
    displayer_thread_.reset();
  }

 public:
  AsyncDisplayer(BaseDisplay* parent,
                 std::ostream* out,
                 Duration interval,
                 bool no_tty)
      : out_(out), parent_(parent), interval_(interval), no_tty_(no_tty) {}
  virtual ~AsyncDisplayer() { done(); }

  /// Set output stream to write to.
  void out(std::ostream* out) {
    assert(out != nullptr);
    out_ = out;
  }

  /// Set parent display
  void parent(BaseDisplay* parent) {
    assert(parent != nullptr);
    parent_ = parent;
  }

  /// Set interval
  void interval(Duration interval) {
    assert(interval >= Duration{0.});
    interval_ = interval;
  }

  /// Get interval
  Duration interval() const { return interval_; }

  /// Set no-tty mode
  void no_tty(bool no_tty) { no_tty_ = no_tty; }

  /// Get no-tty mode
  bool no_tty() const { return no_tty_; }

  /// Notify completion condition variable.
  void notify() { done_cv_.notify_all(); }

  /// Output stream to write to.
  std::ostream& out() { return *out_; }

  /// Return true if the display is running.
  bool running() const { return displayer_thread_ != nullptr; }

  /// Start the display. This starts writing the display in the output stream,
  /// and computing speed if applicable.
  void show() {
    if (running()) { return; } // noop if already show()n before
    assert(interval_ > Duration{0.});

    displayer_thread_ = std::make_unique<std::thread>([this]() {
      display_();
      while (true) {
        bool done = false;
        auto deadline = std::chrono::steady_clock::now() + interval_;
        {
          std::unique_lock lock(done_m_);
          done = done_;
          while (not done and std::chrono::steady_clock::now() < deadline) {
            done_cv_.wait_until(lock, deadline);
            if (std::chrono::steady_clock::now() < deadline) {
              // early wake-up, display again
              if (not no_tty_) { display_(/*redraw=*/true); }
            }
            done = done_;
          }
        }
        display_();
        if (done) {
          // Final newline to avoid overwriting the display with subsequent
          // output
          *out_ << std::endl;
          break;
        }
      }
    });
  }

  /// End the display.
  virtual void done() {
    if (not running()) { return; } // noop if already done() before
    {
      std::lock_guard lock(done_m_);
      done_ = true;
    }
    done_cv_.notify_all();
    join();
  }
};

/// Base class to handle all asynchronous displays.
class BaseDisplay {
 protected:
  std::shared_ptr<AsyncDisplayer> displayer_;
  ///< shared across components of a composite display

  // configuration
  std::string message_;
  std::string format_;

  /// Render a display: animation, progress bar, etc.
  /// @param redraw If true, display is assumed to be redrawn. This, e.g. means
  ///               an Animation should not increment the still frame index.
  /// @param end    String to append at the end of the display. We typically add
  ///               a space to space out the cursor from the content, but for
  ///               composite displays, we don't want extra space between.
  /// @return Number of `\n` characters in the display.
  virtual long render_(bool redraw = false, const std::string& end = " ") = 0;

  /// Display the message to output stream.
  /// @return Number of `\n` characters in the message.
  long render_message_() const {
    if (not message_.empty()) { out() << message_ << " "; }
    return std::count(message_.begin(), message_.end(), '\n');
  }

  BaseDisplay(std::shared_ptr<AsyncDisplayer> displayer)
      : displayer_(displayer) {}

 public:
  BaseDisplay(std::ostream* out = &std::cout,
              Duration interval = Duration{0.5},
              const std::string& message = "",
              const std::string& format = "",
              bool no_tty = false)
      : displayer_(
            std::make_shared<AsyncDisplayer>(this, out, interval, no_tty)),
        message_(message),
        format_(format) {}
  BaseDisplay(const BaseDisplay&) = delete;
  BaseDisplay& operator=(const BaseDisplay&) = delete;

  virtual ~BaseDisplay() { done(); }

  /// Start the display but do not show.
  /// This typically means start measuring speed if applicable, without
  /// displaying anything.
  virtual void start() {}

  /// Start the display.
  virtual void show() {
    start();
    displayer_->show();
  }

  /// End the display.
  virtual void done() { displayer_->done(); }

  /// Output stream to write to.
  std::ostream& out() const { return displayer_->out(); }

  /// Return true if the display is running.
  bool running() const { return displayer_->running(); }

  friend class CompositeDisplay;
  friend class AsyncDisplayer;
};

inline void AsyncDisplayer::display_(bool redraw) {
  static const auto clear_line = "\033[K";
  static const auto cursor_up = "\033[A";

  if (not no_tty_) {
    *out_ << "\r" << clear_line;
    for (long i = 0; i < last_num_newlines_; i++) {
      *out_ << cursor_up << clear_line;
    }
  }
  last_num_newlines_ = parent_->render_(redraw, " ");
  if (no_tty_) { *out_ << "\n"; }
  *out_ << std::flush;
}

/// Animation parameters
struct AnimationConfig {
  std::ostream* out = &std::cout; ///< output stream
  std::string message = "";       ///< message to display before the animation

  /// style as AnimationStyle or custom animation as a list of strings
  std::variant<AnimationStyle, Strings> style = Ellipsis;

  /// interval in which the animation is refreshed
  std::variant<Duration, double> interval = Duration{0.};

  /// no-tty mode if true (no `\r`, slower default refresh)
  bool no_tty = false;

  bool show = true; ///< show the animation immediately after construction
};

inline Duration as_duration(std::variant<Duration, double> interval) {
  if (std::holds_alternative<Duration>(interval)) {
    return std::get<Duration>(interval);
  } else {
    return Duration{std::get<double>(interval)};
  }
}

/// Displays a simple animation with a message.
class AnimationDisplay : public BaseDisplay {
 protected:
  unsigned short frame_ = 0;
  Strings stills_;
  Duration def_interval_{0.5};

  long render_(bool redraw = false, const std::string& end = " ") override {
    long nls = render_message_();
    if (not redraw) { frame_ = (frame_ + 1) % stills_.size(); }
    out() << stills_[frame_] << end;
    return nls; // assuming no newlines in stills
  }

  Duration default_interval_(bool no_tty) const {
    return no_tty ? Duration{60.} : def_interval_;
  }

 public:
  using Style = AnimationStyle;

  /// Constructor.
  /// @param cfg Animation parameters
  AnimationDisplay(const AnimationConfig& cfg = {})
      : BaseDisplay(cfg.out, Duration{0.}, cfg.message, "", cfg.no_tty) {
    if (std::holds_alternative<Strings>(cfg.style)) {
      stills_ = std::get<Strings>(cfg.style);
    } else {
      auto idx =
          static_cast<unsigned short>(std::get<AnimationStyle>(cfg.style));
      auto& stills_pair = animation_stills_[idx];
      stills_ = stills_pair.first;
      def_interval_ = Duration(stills_pair.second);
    }
    frame_ = stills_.size() - 1; // start at the last frame,
                                 // it will be incremented
    displayer_->interval(as_duration(cfg.interval) == Duration{0}
                             ? default_interval_(cfg.no_tty)
                             : as_duration(cfg.interval));

    if (cfg.show) { show(); }
  }

  ~AnimationDisplay() { done(); }
};

/// Convenience factory function to create a shared_ptr to AnimationDisplay.
/// Prefer this to constructing AnimationDisplay directly.
inline auto Animation(const AnimationConfig& cfg = {}) {
  return std::make_shared<AnimationDisplay>(cfg);
}

/// Status is an Animation where it is possible to update the message
/// while the animation is running.
class StatusDisplay : public AnimationDisplay {
 protected:
  std::mutex message_mutex_;

  long render_(bool redraw = false, const std::string& end = " ") override {
    long nls;
    {
      std::lock_guard lock(message_mutex_);
      nls = render_message_();
    }
    if (not redraw) { frame_ = (frame_ + 1) % stills_.size(); }
    out() << stills_[frame_] << end;
    return nls; // assuming no newlines in stills
  }

 public:
  StatusDisplay(const AnimationConfig& cfg = {})
      : AnimationDisplay([&] {
          // Suppress base auto-show so that the display thread only starts
          // once StatusDisplay's vtable (and its synchronized render_) is in
          // place.
          AnimationConfig c = cfg;
          c.show = false;
          return c;
        }()) {
    if (cfg.show) { show(); }
  }
  ~StatusDisplay() { done(); }

  /// Update the displayed message.
  /// This is synchronized between the display thread and the calling thread.
  void message(const std::string& message) {
    {
      std::lock_guard lock(message_mutex_);
      message_ = message;
    }
    // notify the display thread to trigger a redraw
    displayer_->notify();
  }

  /// Get the current message.
  std::string message() {
    std::lock_guard lock(message_mutex_);
    return message_;
  }
};

/// Convenience factory function to create a shared_ptr to StatusDisplay.
/// Prefer this to constructing StatusDisplay directly.
inline auto Status(const AnimationConfig& cfg = {}) {
  return std::make_shared<StatusDisplay>(cfg);
}

template <typename T>
struct Provider {
  using value_type = T;
  using provider_type = T*;
  using underlying_type = T;

  Provider(provider_type ptr) noexcept : ptr_{ptr} {}
  Provider(const Provider&) = default;
  Provider& operator=(const Provider&) = default;

  [[nodiscard]] T load() const noexcept { return *ptr_; }

  [[nodiscard]] bool ok() const noexcept { return ptr_; }

  provider_type ptr_{nullptr};
};

template <typename T>
struct Provider<std::atomic<T>> {
  using value_type = T;
  using provider_type = std::atomic<T>*;
  using underlying_type = std::atomic<T>;

  Provider(provider_type ptr) noexcept : ptr_{ptr} {}
  Provider(const Provider&) = default;
  Provider& operator=(const Provider&) = default;

  [[nodiscard]] T load() const noexcept {
    return ptr_->load(std::memory_order_relaxed);
  }

  [[nodiscard]] bool ok() const noexcept { return ptr_; }

  provider_type ptr_{nullptr};
};

template <typename T>
struct Provider<std::function<T()>> {
  using value_type = T;
  using provider_type = std::function<T()>;
  using underlying_type = std::function<T()>;

  Provider(provider_type fun) : fun_{std::move(fun)} {}
  Provider(const Provider&) = default;
  Provider& operator=(const Provider&) = default;

  [[nodiscard]] T load() const { return fun_(); }

  [[nodiscard]] bool ok() const noexcept { return fun_; }

  provider_type fun_;
};

template <typename T, typename = void>
struct ReasonablyInvocable {
  constexpr static auto value = false;
};

template <typename T>
struct ReasonablyInvocable<
    T,
    std::enable_if_t<!std::is_same_v<void, std::invoke_result_t<T>>>> {
  constexpr static auto value = true;
};

template <typename T>
constexpr bool ReasonablyInvocableV = ReasonablyInvocable<T>::value;

template <typename T, typename = void>
struct ProviderSelector {
  using type_t = Provider<T>;
};

template <typename T>
struct ProviderSelector<std::atomic<T>> {
  using type_t = Provider<std::atomic<T>>;
};

template <typename T>
struct ProviderSelector<
    T,
    std::enable_if_t<!std::is_same_v<void, std::invoke_result_t<T>>>> {
  using type_t = Provider<std::function<std::invoke_result_t<T>()>>;
};

template <typename T>
using provider_t = typename ProviderSelector<T>::type_t;

/// Trait class to extract underlying value type from numerics and
/// std::atomics of numerics.
template <typename T>
struct AtomicTraits {
  using value_type = T;
};

template <typename T>
struct AtomicTraits<std::atomic<T>> {
  using value_type = T;
};

template <typename T>
struct AtomicTraits<Provider<T>> {
  using value_type = typename Provider<T>::value_type;
};

template <typename T>
struct AtomicTraits<Provider<std::atomic<T>>> {
  using value_type = typename Provider<std::atomic<T>>::value_type;
};

template <typename T>
struct AtomicTraits<Provider<std::function<T()>>> {
  using value_type = typename Provider<std::function<T()>>::value_type;
};

template <typename T>
using value_t = typename AtomicTraits<T>::value_type;

template <typename T>
using signed_t = typename std::conditional_t<std::is_integral_v<T>,
                                             std::make_signed<T>,
                                             std::common_type<T>>::type;

/// Helper class to measure and display speed of progress.
template <typename ProgressProvider>
class Speedometer {
 private:
  const ProgressProvider& progress_provider_; // Current amount of work done
  double discount_;

  using ValueType = value_t<ProgressProvider>;
  using SignedType = signed_t<ValueType>;
  using Clock = std::chrono::steady_clock;
  using Time = std::chrono::time_point<Clock>;

  double progress_increment_sum_ = 0; // (weighted) sum of progress increments
  Duration duration_increment_sum_{}; // (weighted) sum of inter-increment gaps

  Time last_increment_time_;
  ValueType last_progress_{};

 public:
  double speed() {
    Time now = Clock::now();

    ValueType progress_copy =
        progress_provider_.load(); // to avoid progress_ changing below
    SignedType progress_increment =
        SignedType(progress_copy) - SignedType(last_progress_);

    if (progress_increment != 0) {
      Duration gap = now - last_increment_time_;
      progress_increment_sum_ =
          (1 - discount_) * progress_increment_sum_ + progress_increment;
      duration_increment_sum_ = (1 - discount_) * duration_increment_sum_ + gap;
      last_progress_ = progress_copy;
      last_increment_time_ = now;
    }

    if (duration_increment_sum_.count() == 0) { return 0; }
    double base_speed =
        progress_increment_sum_ / duration_increment_sum_.count();

    // If we've been waiting longer than the EWMA inter-increment gap implies,
    // the true speed must be at most 1/current_wait in magnitude, otherwise
    // we would have already seen another increment. Clamp accordingly so the
    // reported speed decays when increments stop.
    Duration current_wait = now - last_increment_time_;
    if (std::abs(base_speed) * current_wait.count() > 1.0) {
      return std::copysign(1.0 / current_wait.count(), base_speed);
    }
    return base_speed;
  }

  /// Write speed to given output stream. Speed is a double (written with
  /// precision 2), possibly followed by a unit of speed.
  void render_speed(std::ostream* out,
                    const std::string& speed_unit,
                    const std::string& end = " ") {
    std::stringstream ss; // use local stream to avoid disturbing `out` with
                          // std::fixed and std::setprecision
    double speed = this->speed();
    ss << std::fixed << std::setprecision(2) << "(" << speed;
    if (speed_unit.empty()) {
      ss << ")";
    } else {
      ss << " " << speed_unit << ")";
    }
    ss << end;

    auto s = ss.str();
    *out << s;
  }

  /// Start computing the speed based on time between progress increments.
  void start() {
    last_progress_ = progress_provider_.load();
    last_increment_time_ = Clock::now();
  }

  /// Constructor.
  /// @param progress Reference to numeric to measure the change of.
  /// @param discount Discount factor in [0, 1] to use in computing the speed.
  ///                 Previous increments are weighted by (1-discount).
  ///                 If discount is 0, all increments are weighted equally.
  ///                 If discount is 1, only the most recent increment is
  ///                 considered.
  Speedometer(const ProgressProvider& progress_provider, double discount)
      : progress_provider_(progress_provider), discount_(discount) {
    if (discount < 0 or discount > 1) {
      throw std::runtime_error("Discount must be in [0, 1]");
    }
  }
};

/// Counter parameters
struct CounterConfig {
  std::ostream* out = &std::cout; ///< output stream
  std::string format = "";        ///< format string to format entire counter
  std::string message = "";       ///< message to display with the counter

  /// Speed discount factor in [0, 1] to use in computing the speed.
  /// Previous increments are weighed by (1-speed).
  /// If speed is 0, all increments are weighed equally.
  /// If speed is 1, only the most recent increment is
  /// considered. If speed is `std::nullopt`, speed is not computed.
  std::optional<double> speed = std::nullopt;

  std::string speed_unit = "it/s"; ///< unit of speed text next to speed

  /// interval in which the counter is refreshed
  std::variant<Duration, double> interval = Duration{0.};
  bool no_tty = false; ///< no-tty mode if true (no \r, slower default refresh)
  bool show = true;    ///< show the counter immediately after construction
};

/// Monitors and displays a single numeric variable
template <typename Progress = size_t>
class CounterDisplay : public BaseDisplay {
 protected:
  using ProgressProvider = provider_t<Progress>;
  ProgressProvider progress_provider_; // current amount of work done
  std::unique_ptr<Speedometer<ProgressProvider>> speedom_;
  std::string speed_unit_ = "it/s"; // unit of speed text next to speed

  std::stringstream ss_;

 protected:
  /// Write the value of progress to the output stream
  void render_counts_(const std::string& end = " ") {
    ss_ << progress_provider_.load();
    out() << ss_.str() << end;
    ss_.str("");
  }

  /// Write the value of progress with the message to the output stream
  long render_(bool /*redraw*/ = false, const std::string& end = " ") override {
#if defined(BARKEEP_ENABLE_FMT_FORMAT)
    if (not format_.empty()) {
      using namespace fmt::literals;
      value_t<Progress> progress = progress_provider_.load();
      if (speedom_) {
        fmt::print(out(),
                   fmt::runtime(format_),
                   "value"_a = progress,
                   "speed"_a = speedom_->speed(),
                   "red"_a = red,
                   "green"_a = green,
                   "yellow"_a = yellow,
                   "blue"_a = blue,
                   "magenta"_a = magenta,
                   "cyan"_a = cyan,
                   "reset"_a = reset);
      } else {
        fmt::print(out(),
                   fmt::runtime(format_),
                   "value"_a = progress,
                   "red"_a = red,
                   "green"_a = green,
                   "yellow"_a = yellow,
                   "blue"_a = blue,
                   "magenta"_a = magenta,
                   "cyan"_a = cyan,
                   "reset"_a = reset);
      }
      return std::count(format_.begin(), format_.end(), '\n');
    }
#elif defined(BARKEEP_ENABLE_STD_FORMAT)
    if (not format_.empty()) {
      value_t<Progress> progress = progress_provider_.load();
      auto speed = speedom_ ? speedom_->speed() : std::nan("");
      out() << std::vformat(format_,
                            std::make_format_args(progress,
                                                  speed,   // 1
                                                  red,     // 2
                                                  green,   // 3
                                                  yellow,  // 4
                                                  blue,    // 5
                                                  magenta, // 6
                                                  cyan,    // 7
                                                  reset)   // 8

      );
      return std::count(format_.begin(), format_.end(), '\n');
    }
#endif
    long nls = render_message_();
    render_counts_(speedom_ ? " " : end);
    if (speedom_) { speedom_->render_speed(&out(), speed_unit_, end); }
    nls += std::count(end.begin(), end.end(), '\n');
    return nls;
  }

  /// Default interval in which the display is refreshed, if not provided.
  Duration default_interval_(bool no_tty) const {
    return no_tty ? Duration{60.} : Duration{.1};
  }

  void start() override {
    if constexpr (std::is_floating_point_v<value_t<ProgressProvider>>) {
      ss_ << std::fixed << std::setprecision(2);
    }
    if (speedom_) { speedom_->start(); }
  }

 public:
  /// Constructor.
  /// @param progress Variable to be monitored and displayed
  /// @param cfg      Counter parameters
  CounterDisplay(provider_t<Progress> progress_provider,
                 const CounterConfig& cfg = {})
      : BaseDisplay(cfg.out,
                    as_duration(cfg.interval),
                    cfg.message,
                    cfg.format.empty() ? "" : cfg.format + " ",
                    cfg.no_tty),
        progress_provider_(std::move(progress_provider)),
        speed_unit_(cfg.speed_unit) {
    if (cfg.speed) {
      speedom_ = std::make_unique<Speedometer<ProgressProvider>>(
          progress_provider_, *cfg.speed);
    }
    if (displayer_->interval() == Duration{0.}) {
      displayer_->interval(default_interval_(cfg.no_tty));
    }
    if (cfg.show) { show(); }
  }

  CounterDisplay(Progress* progress_provider, const CounterConfig& cfg = {})
      : CounterDisplay(provider_t<Progress>(progress_provider), cfg) {}

  CounterDisplay(Progress progress_provider, const CounterConfig& cfg = {})
      : CounterDisplay(provider_t<Progress>(std::move(progress_provider)),
                       cfg) {}

  ~CounterDisplay() { done(); }
};

/// Convenience factory function to create a shared_ptr to CounterDisplay.
/// Prefer this to constructing CounterDisplay directly.
template <
    typename Progress,
    typename ProgressProvider = provider_t<Progress>,
    typename std::enable_if_t<ReasonablyInvocableV<Progress>, bool> = true>
auto Counter(Progress&& progress, const CounterConfig& cfg = {}) {
  return std::make_shared<CounterDisplay<Progress>>(
      std::forward<Progress>(progress), cfg);
}

template <typename Progress, typename ProgressProvider = provider_t<Progress>>
auto Counter(Progress* progress, const CounterConfig& cfg = {}) {
  return std::make_shared<CounterDisplay<Progress>>(progress, cfg);
}

/// Progress bar parameters
template <typename ValueType>
struct ProgressBarConfig {
  std::ostream* out = &std::cout; ///< output stream
  ValueType total = 100;          ///< total amount of work for a full bar.
                                  ///< A total of 0 is immediately complete.
  std::string format = "";        ///< format string for the entire progress bar
  std::string message = "";       ///< message to display with the bar

  /// Speed discount factor in [0, 1] to use in computing the speed.
  /// Previous increments are weighted by (1-speed).
  /// If speed is 0, all increments are weighed equally.
  /// If speed is 1, only the most recent increment is
  /// considered. If speed is `std::nullopt`, speed is not computed.
  std::optional<double> speed = std::nullopt;

  std::string speed_unit = "it/s"; ///< unit of speed text next to speed

  /// progress bar style, or custom style as BarParts
  std::variant<ProgressBarStyle, BarParts> style = Blocks;

  /// interval in which the progress bar is refreshed
  std::variant<Duration, double> interval = Duration{0.};
  bool no_tty = false; ///< no-tty mode if true (no \r, slower default refresh)
  bool show = true;    ///< show the progress bar immediately after construction
};

/// Displays a progress bar, by comparing the progress value being monitored to
/// a given total value. Optionally reports speed.
template <typename Progress>
class ProgressBarDisplay : public BaseDisplay {
 protected:
  using ProgressProvider = provider_t<Progress>;
  using ValueType = value_t<ProgressProvider>;

  ProgressProvider progress_provider_; // work done so far
  std::unique_ptr<Speedometer<ProgressProvider>> speedom_;
  std::string speed_unit_ = "it/s";    // unit of speed text next to speed
  static constexpr size_t width_ = 30; // width of progress bar
                                       // (TODO: make customizable?)
  ValueType total_{100};               // total work

  BarParts bar_parts_; // progress bar drawing strings

 protected:
  /// Compute the shape of the progress bar based on progress and write to
  /// output stream.
  void render_progress_bar_(std::ostream* out) {
    ValueType progress_copy =
        progress_provider_.load(); // to avoid progress_ changing
                                   // during computations below
    bool complete = progress_copy >= total_;
    int on = (total_ == ValueType(0))
                 ? int(width_)
                 : int(ValueType(width_) * progress_copy / total_);
    size_t partial =
        (total_ == ValueType(0))
            ? 0
            : size_t(ValueType(bar_parts_.fill.size()) * ValueType(width_) *
                         progress_copy / total_ -
                     ValueType(bar_parts_.fill.size()) * ValueType(on));
    if (on >= int(width_)) {
      on = width_;
      partial = 0;
    } else if (on < 0) {
      on = 0;
      partial = 0;
    }
    assert(partial < bar_parts_.fill.size());
    auto off = width_ - size_t(on) - size_t(partial > 0);

    // draw progress bar

    // left end
    *out << bar_parts_.left;
    *out << (complete ? bar_parts_.complete_left_modifier
                      : bar_parts_.incomplete_left_modifier);

    // filled portion
    for (int i = 0; i < on; i++) { *out << bar_parts_.fill.back(); }

    // partially filled character
    if (partial > 0) { *out << bar_parts_.fill.at(partial - 1); }

    *out << bar_parts_.middle_modifier;

    // partially empty character if available
    if (off > 0) {
      if (bar_parts_.empty.size() > 1) {
        assert(bar_parts_.empty.size() == bar_parts_.fill.size());
        *out << bar_parts_.empty.at(partial);
      } else {
        *out << bar_parts_.empty.back();
      }
    }

    // empty portion
    for (size_t i = 1; i < off; i++) { *out << bar_parts_.empty.back(); }

    // right end
    *out << bar_parts_.right_modifier;
    *out << bar_parts_.right;
  }

  /// Write progress value with the total, e.g. 50/100, to output stream.
  /// Progress width is expanded (and right justified) to match width of total.
  void render_counts_(const std::string& end = " ") {
    std::stringstream ss, totals;
    if constexpr (std::is_floating_point_v<value_t<ProgressProvider>>) {
      ss << std::fixed << std::setprecision(2);
      totals << std::fixed << std::setprecision(2);
    }
    totals << total_;
    auto width = static_cast<std::streamsize>(totals.str().size());
    ss.width(width);
    ss << std::right << progress_provider_.load() << "/" << total_ << end;
    out() << ss.str();
  }

  /// Write the percent completed to output stream
  void render_percentage_(const std::string& end = " ") {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss.width(6);
    double percent = (total_ == ValueType(0))
                         ? 100.
                         : progress_provider_.load() * 100. / total_;
    ss << std::right << percent << "%" << end;
    out() << ss.str();
  }

  /// Run all of the individual render methods to write everything to stream
  long render_(bool /*redraw*/ = false, const std::string& end = " ") override {
#if defined(BARKEEP_ENABLE_FMT_FORMAT)
    if (not format_.empty()) {
      using namespace fmt::literals;
      value_t<Progress> progress = progress_provider_.load();

      std::stringstream bar_ss;
      render_progress_bar_(&bar_ss);

      double percent =
          (total_ == value_t<Progress>(0)) ? 100. : progress * 100. / total_;

      if (speedom_) {
        fmt::print(out(),
                   fmt::runtime(format_),
                   "value"_a = progress,
                   "bar"_a = bar_ss.str(),
                   "percent"_a = percent,
                   "total"_a = total_,
                   "speed"_a = speedom_->speed(),
                   "red"_a = red,
                   "green"_a = green,
                   "yellow"_a = yellow,
                   "blue"_a = blue,
                   "magenta"_a = magenta,
                   "cyan"_a = cyan,
                   "reset"_a = reset);
      } else {
        fmt::print(out(),
                   fmt::runtime(format_),
                   "value"_a = progress,
                   "bar"_a = bar_ss.str(),
                   "percent"_a = percent,
                   "total"_a = total_,
                   "red"_a = red,
                   "green"_a = green,
                   "yellow"_a = yellow,
                   "blue"_a = blue,
                   "magenta"_a = magenta,
                   "cyan"_a = cyan,
                   "reset"_a = reset);
      }
      return std::count(format_.begin(), format_.end(), '\n');
    }
#elif defined(BARKEEP_ENABLE_STD_FORMAT)
    if (not format_.empty()) {
      value_t<Progress> progress = progress_provider_.load();

      std::stringstream bar_ss;
      render_progress_bar_(&bar_ss);
      std::string bar = bar_ss.str();

      double percent =
          (total_ == value_t<Progress>(0)) ? 100. : progress * 100. / total_;
      auto speed = speedom_ ? speedom_->speed() : std::nan("");

      out() << std::vformat(format_,
                            std::make_format_args(progress, // 0
                                                  bar,      // 1
                                                  percent,  // 2
                                                  total_,   // 3
                                                  speed,    // 4
                                                  red,      // 5
                                                  green,    // 6
                                                  yellow,   // 7
                                                  blue,     // 8
                                                  magenta,  // 9
                                                  cyan,     // 10
                                                  reset));  // 11
      return std::count(format_.begin(), format_.end(), '\n');
    }
#endif
    auto& out = this->out();
    long nls = render_message_();

    out << bar_parts_.percent_left_modifier;
    render_percentage_();
    out << bar_parts_.percent_right_modifier;

    render_progress_bar_(&out);
    out << " ";

    out << bar_parts_.value_left_modifier;
    render_counts_(speedom_ ? " " : end);
    out << bar_parts_.value_right_modifier;

    if (speedom_) {
      out << bar_parts_.speed_left_modifier;
      speedom_->render_speed(&out, speed_unit_, end);
      out << bar_parts_.speed_right_modifier;
    }

    nls += std::count(end.begin(), end.end(), '\n');

    // assumes no newlines in bar parts
    return nls;
  }

  Duration default_interval_(bool no_tty) const {
    return no_tty ? Duration{60.} : Duration{.1};
  }

  void start() override {
    if (speedom_) { speedom_->start(); }
  }

 public:
  using Style = ProgressBarStyle;

  /// Constructor.
  /// @param progress Variable to be monitored to measure completion
  /// @param cfg      ProgressBar parameters
  ProgressBarDisplay(provider_t<Progress> progress_provider,
                     const ProgressBarConfig<ValueType>& cfg = {})
      : BaseDisplay(cfg.out,
                    as_duration(cfg.interval),
                    cfg.message,
                    cfg.format.empty() ? "" : cfg.format + " ",
                    cfg.no_tty),
        progress_provider_(std::move(progress_provider)),
        speed_unit_(cfg.speed_unit),
        total_(cfg.total) {
    if (std::holds_alternative<BarParts>(cfg.style)) {
      bar_parts_ = std::get<BarParts>(cfg.style);
    } else {
      bar_parts_ = progress_bar_parts_[static_cast<unsigned short>(
          std::get<ProgressBarStyle>(cfg.style))];
    }
    if (cfg.speed) {
      speedom_ = std::make_unique<Speedometer<ProgressProvider>>(
          progress_provider_, *cfg.speed);
    }
    if (displayer_->interval() == Duration{0.}) {
      displayer_->interval(default_interval_(cfg.no_tty));
    }
    if (cfg.show) { show(); }
  }

  ProgressBarDisplay(Progress* progress_provider,
                     const ProgressBarConfig<ValueType>& cfg = {})
      : ProgressBarDisplay(provider_t<Progress>(progress_provider), cfg) {}

  ProgressBarDisplay(Progress progress_provider,
                     const ProgressBarConfig<ValueType>& cfg = {})
      : ProgressBarDisplay(provider_t<Progress>(std::move(progress_provider)),
                           cfg) {}

  ~ProgressBarDisplay() { done(); }
};

/// Convenience factory function to create a shared_ptr to ProgressBarDisplay.
/// Prefer this to constructing ProgressBarDisplay directly.
template <
    typename Progress,
    typename ProgressProvider = provider_t<Progress>,
    typename std::enable_if_t<ReasonablyInvocableV<Progress>, bool> = true>
auto ProgressBar(Progress&& progress,
                 const ProgressBarConfig<value_t<ProgressProvider>>& cfg = {}) {
  return std::make_shared<ProgressBarDisplay<Progress>>(
      std::forward<Progress>(progress), cfg);
}

template <typename Progress, typename ProgressProvider = provider_t<Progress>>
auto ProgressBar(Progress* progress,
                 const ProgressBarConfig<value_t<ProgressProvider>>& cfg = {}) {
  return std::make_shared<ProgressBarDisplay<Progress>>(progress, cfg);
}

/// Creates a composite display out of multiple child displays to show
/// them together.
/// For instance, you can combine two Counter objects, or a Counter and a
/// ProgressBar to concurrently monitor two variables.
class CompositeDisplay : public BaseDisplay {
 protected:
  std::string delim_ = " "; ///< delimiter between displays
  std::vector<std::shared_ptr<BaseDisplay>> displays_;
  ///< child displays to show

  long render_(bool redraw = false, const std::string& end = " ") override {
    long nls = render_message_();
    for (auto it = displays_.begin(); it != displays_.end(); it++) {
      if (it != displays_.begin()) {
        out() << delim_;
        nls += std::count(delim_.begin(), delim_.end(), '\n');
      }
      (*it)->render_(redraw, (it != displays_.end() - 1) ? "" : end);
    }
    nls += std::count(end.begin(), end.end(), '\n');
    return nls;
  }

  void start() override {
    for (auto& display : displays_) { display->start(); }
  }

 public:
  CompositeDisplay(const std::vector<std::shared_ptr<BaseDisplay>>& displays,
                   std::string delim = " ")
      : delim_(std::move(delim)), displays_(displays) {
    if (displays_.empty()) {
      throw std::runtime_error("Cannot create composite from empty displays!");
    }
    for (auto& display : displays_) {
      if (display->displayer_->running()) {
        throw std::runtime_error("Cannot combine running displays!");
      }
    }
    displayer_ = displays.front()->displayer_;
    for (auto& display : displays_) {
      // all children need to point to same AsyncDisplayer
      auto& front = displays.front()->displayer_;
      front->interval(
          std::min(front->interval(), display->displayer_->interval()));
      front->no_tty(front->no_tty() or display->displayer_->no_tty());

      display->displayer_->out(&front->out());
    }
    displayer_->parent(this);
  }

  ~CompositeDisplay() {
    done();
    // Restore the front child as the shared displayer's parent, in case
    // children outlive this composite.
    displayer_->parent(displays_.front().get());
  }
};

/// Convenience factory function to create a shared_ptr to CompositeDisplay.
/// Prefer this to constructing CompositeDisplay directly.
inline auto Composite(const std::vector<std::shared_ptr<BaseDisplay>>& displays,
                      std::string delim = " ") {
  return std::make_shared<CompositeDisplay>(displays, std::move(delim));
}

/// Pipe operator can be used to combine two displays into a Composite.
inline auto operator|(std::shared_ptr<BaseDisplay> left,
                      std::shared_ptr<BaseDisplay> right) {
  return std::make_shared<CompositeDisplay>(
      std::vector{std::move(left), std::move(right)});
}

/// IterableBar parameters
template <typename ValueType>
struct IterableBarConfig {
  std::ostream* out = &std::cout; ///< output stream
  std::string format = "";        ///< format string for the entire progress bar
  std::string message = "";       ///< message to display with the bar

  /// Speed discount factor in [0, 1] to use in computing the speed.
  /// Previous increments are weighted by (1-speed).
  /// If speed is 0, all increments are weighed equally.
  /// If speed is 1, only the most recent increment is
  /// considered. If speed is `std::nullopt`, speed is not computed.
  std::optional<double> speed = std::nullopt;

  std::string speed_unit = "it/s"; ///< unit of speed text next to speed
  ProgressBarStyle style = Blocks; ///< style of progress bar

  /// interval in which the progress bar is refreshed
  std::variant<Duration, double> interval = Duration{0.};
  bool no_tty = false; ///< no-tty mode if true (no \r, slower default refresh)
};

/// A progress bar that can be used with range-based for loops, that
/// automatically tracks the progress of the loop.
///
/// IterableBar starts the display not at the time of construction, but at the
/// time of the first call to begin(). Thus, it is possible to set it up prior
/// to loop execution.
///
/// Similarly, it ends the display not at the time of destruction, but at the
/// first increment of the iterator past the end. Thus, even if the object stays
/// alive after the loop, the display will be stopped.
template <typename Container>
class IterableBar {
 public:
  using ProgressType = std::atomic<size_t>;
  using ValueType = value_t<ProgressType>;
  using Bar = ProgressBarDisplay<ProgressType>;

 private:
  Container& container_;
  std::shared_ptr<ProgressType> idx_;
  std::shared_ptr<Bar> bar_;

 public:
  /// IterableBar's iterator class that wraps the container's iterator.
  /// When incremented, it increments the progress index.
  class Iterator {
   private:
    typename Container::iterator it_, end_;
    ProgressType& idx_;
    std::shared_ptr<Bar> bar_;

   public:
    Iterator(typename Container::iterator it,
             typename Container::iterator end,
             ProgressType& idx,
             std::shared_ptr<Bar> bar)
        : it_(it), end_(end), idx_(idx), bar_(std::move(bar)) {}

    Iterator& operator++() {
      it_++;
      idx_++;
      if (it_ == end_) { bar_->done(); }
      return *this;
    }

    bool operator!=(const Iterator& other) const { return it_ != other.it_; }

    auto& operator*() { return *it_; }
  };

  IterableBar(Container& container,
              const IterableBarConfig<ValueType>& cfg = {})
      : container_(container),
        idx_(std::make_shared<ProgressType>(0)),
        bar_(std::make_shared<Bar>(
            &*idx_,
            ProgressBarConfig<ValueType>{cfg.out,
                                         container.size(),
                                         cfg.format,
                                         cfg.message,
                                         cfg.speed,
                                         cfg.speed_unit,
                                         cfg.style,
                                         cfg.interval,
                                         cfg.no_tty,
                                         /*show=*/false})) {}

  auto begin() {
    bar_->show();
    return Iterator(container_.begin(), container_.end(), *idx_, bar_);
  }

  auto end() {
    return Iterator(container_.end(), container_.end(), *idx_, bar_);
  }
};

} // namespace barkeep

#endif
