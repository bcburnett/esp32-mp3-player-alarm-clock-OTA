    <script type="module">
      class HowtoTooltip extends HTMLElement {
        constructor() {
          super();
          this._show = this._show.bind(this);
          this._hide = this._hide.bind(this);
        }
        connectedCallback() {
          if (!this.hasAttribute("role")) this.setAttribute("role", "tooltip");

          if (!this.hasAttribute("tabindex")) this.setAttribute("tabindex", -1);
          this.style.position = "absolute";
          this.style.background= "white";

          this._hide();
          this._target = document.querySelector(
            "[aria-describedby=" + this.id + "]"
          );
          var rect = this._target.getBoundingClientRect();

          this.style.top = rect.top;
          this.style.left = rect.left;
          if (!this._target) return;
          this._target.addEventListener("focus", this._show);
          this._target.addEventListener("blur", this._hide);
          this._target.addEventListener("mouseenter", this._show);
          this._target.addEventListener("mouseleave", this._hide);
        }
        disconnectedCallback() {
          if (!this._target) return;
          this._target.removeEventListener("focus", this._show);
          this._target.removeEventListener("blur", this._hide);
          this._target.removeEventListener("mouseenter", this._show);
          this._target.removeEventListener("mouseleave", this._hide);
          this._target = null;
        }
        _show() {
          this.hidden = false;
        }

        _hide() {
          this.hidden = true;
        }
      }
      window.customElements.define("howto-tooltip", HowtoTooltip);
    </script>