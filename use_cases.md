## I. Use Cases Liên Quan Đến Xác Thực và Quản Lý Tài Khoản

1.  **UC-AUTH-01: End-user tự đăng ký tài khoản**
    * **Actor:** End-user chưa có tài khoản.
    * **Mô tả:** End-user cung cấp thông tin (username, password, họ tên, email/SĐT) qua giao diện dòng lệnh. Hệ thống kiểm tra tính hợp lệ (username không trùng), lưu thông tin (mật khẩu được băm), tạo bản ghi người dùng và ví điểm tương ứng.

2.  **UC-AUTH-02: Administrator tạo tài khoản cho End-user**
    * **Actor:** Administrator.
    * **Mô tả:** Administrator sử dụng chức năng tạo tài khoản, nhập thông tin do End-user cung cấp. Hệ thống cho phép Administrator nhập mật khẩu hoặc tự sinh mật khẩu ngẫu nhiên (và thông báo mật khẩu này). Hệ thống lưu thông tin, tạo bản ghi người dùng và ví điểm.

3.  **UC-AUTH-03: End-user đăng nhập**
    * **Actor:** End-user đã có tài khoản.
    * **Mô tả:** End-user nhập username và password. Hệ thống tìm kiếm user, băm mật khẩu nhập vào, so sánh với hash đã lưu. Nếu hợp lệ, cho phép truy cập các chức năng tương ứng với vai trò.
    * **Luồng phụ:** Nếu đăng nhập thành công và mật khẩu đang ở trạng thái "tự sinh", hệ thống chuyển ngay đến UC-AUTH-05.

4.  **UC-AUTH-04: End-user đăng xuất**
    * **Actor:** End-user đang đăng nhập.
    * **Mô tả:** End-user chọn chức năng đăng xuất. Hệ thống xóa trạng thái đăng nhập hiện tại và quay về màn hình chính.

5.  **UC-AUTH-05: End-user thay đổi mật khẩu lần đầu (bắt buộc)**
    * **Actor:** End-user đang đăng nhập (lần đầu với mật khẩu tự sinh).
    * **Mô tả:** Ngay sau khi đăng nhập thành công (UC-AUTH-03), hệ thống yêu cầu End-user nhập mật khẩu mới và xác nhận. Hệ thống cập nhật mật khẩu (băm) và trạng thái mật khẩu. Không yêu cầu OTP cho lần đổi bắt buộc này (hoặc có thể yêu cầu tùy thiết kế).

6.  **UC-AUTH-06: End-user tự thay đổi mật khẩu**
    * **Actor:** End-user đang đăng nhập.
    * **Mô tả:** End-user chọn chức năng đổi mật khẩu. Hệ thống yêu cầu nhập mật khẩu cũ, mật khẩu mới và xác nhận. Sau khi xác thực mật khẩu cũ, hệ thống yêu cầu xác thực OTP. Nếu OTP hợp lệ, hệ thống cập nhật mật khẩu (băm) và trạng thái.

7.  **UC-AUTH-07: Hệ thống xác thực OTP**
    * **Actor:** Hệ thống (được kích hoạt bởi các use case khác).
    * **Mô tả:** Khi một hành động quan trọng (đổi mật khẩu, sửa thông tin, chuyển điểm) sắp diễn ra, hệ thống tạo một mã OTP, hiển thị/gửi cho End-user, yêu cầu End-user nhập lại. Hệ thống xác minh mã nhập vào có trùng khớp và hợp lệ không.

## II. Use Cases Liên Quan Đến Quản Lý Thông Tin Cá Nhân

8.  **UC-INFO-01: End-user xem thông tin cá nhân**
    * **Actor:** End-user đang đăng nhập.
    * **Mô tả:** End-user chọn chức năng xem thông tin. Hệ thống hiển thị các thông tin liên quan đến tài khoản của End-user đó (username, họ tên, email/SĐT, vai trò, mã ví).

9.  **UC-INFO-02: End-user chỉnh sửa thông tin cá nhân**
    * **Actor:** End-user đang đăng nhập.
    * **Mô tả:** End-user chọn chức năng chỉnh sửa. End-user chọn trường muốn sửa (Họ tên, Email/SĐT), nhập giá trị mới. Hệ thống yêu cầu xác thực OTP. Nếu OTP hợp lệ, hệ thống cập nhật thông tin.

10. **UC-INFO-03: Administrator xem danh sách End-user**
    * **Actor:** Administrator.
    * **Mô tả:** Administrator chọn chức năng xem danh sách. Hệ thống hiển thị danh sách các tài khoản trong hệ thống (có thể gồm username, họ tên, vai trò).

11. **UC-INFO-04: Administrator chỉnh sửa thông tin End-user khác**
    * **Actor:** Administrator.
    * **Mô tả:** Administrator chọn chức năng sửa, nhập username của End-user cần sửa. Administrator chọn trường muốn sửa (Họ tên, Email/SĐT), nhập giá trị mới. Hệ thống yêu cầu Administrator lấy OTP từ *chủ tài khoản* (End-user đó) và nhập vào. Nếu OTP hợp lệ, hệ thống cập nhật thông tin cho End-user đó.

## III. Use Cases Liên Quan Đến Quản Lý Ví Điểm Thưởng

12. **UC-WALLET-01: End-user xem số dư ví**
    * **Actor:** End-user đang đăng nhập.
    * **Mô tả:** End-user chọn chức năng xem số dư. Hệ thống truy xuất và hiển thị số điểm hiện có trong ví của End-user.

13. **UC-WALLET-02: End-user xem lịch sử giao dịch**
    * **Actor:** End-user đang đăng nhập.
    * **Mô tả:** End-user chọn chức năng xem lịch sử. Hệ thống truy xuất và hiển thị danh sách các giao dịch (chuyển đi, nhận về) liên quan đến ví của End-user, bao gồm thông tin chi tiết (ID giao dịch, thời gian, ví gửi/nhận, số tiền, trạng thái).

14. **UC-WALLET-03: End-user chuyển điểm thưởng**
    * **Actor:** End-user đang đăng nhập.
    * **Mô tả:** End-user chọn chức năng chuyển điểm. End-user nhập mã ví người nhận và số điểm muốn chuyển. Hệ thống kiểm tra mã ví nhận có tồn tại và số dư khả dụng. Hệ thống yêu cầu xác thực OTP. Nếu OTP hợp lệ và đủ số dư, hệ thống thực hiện giao dịch (trừ điểm ví gửi, cộng điểm ví nhận một cách atomic) và ghi nhận vào lịch sử giao dịch.

## IV. Use Cases Hệ thống (ngầm định)

* **UC-SYS-01:** Hệ thống lưu trữ dữ liệu người dùng và ví vào database.
* **UC-SYS-02:** Hệ thống hash và lưu trữ mật khẩu an toàn (sử dụng salt).
* **UC-SYS-03:** Hệ thống đảm bảo tính duy nhất của username và mã ví.
* **UC-SYS-04:** Hệ thống đảm bảo tính atomic cho giao dịch chuyển điểm.
* **UC-SYS-05:** Hệ thống ghi log mọi giao dịch (kể cả thất bại).
* **UC-SYS-06:** Hệ thống thực hiện sao lưu dữ liệu định kỳ hoặc khi có thay đổi quan trọng.