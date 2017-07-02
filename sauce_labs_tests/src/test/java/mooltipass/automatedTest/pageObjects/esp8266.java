package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class esp8266 extends AbstractPage{
	
	public esp8266 (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//a[contains(text(),'Login')]")
	private WebElement loginBtn;
	
	@FindBy(id = "username")
	private WebElement email;

	@FindBy(id = "password")
	private WebElement password;
	
	@FindBy(xpath = "//button[contains(text(),'Log me in')]")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//div[@class='navbar-footer-content']//a[contains(text(),'Logout')]")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//ul[@class='nav navbar-nav navbar-right navbar-nav-fancy']//li[@class='dropdown dropdown-avatar']")
	private WebElement dashBoard;
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		driver.get("http://www.esp8266.com/ucp.php");
	}
	
	public void submit(){
	submitLogin.click();
	}
	
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		dashBoard.click();
		
	}
	public void logout(){
		waitUntilAppears(logoutBtn);
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		return isElementPresent(By.xpath("//ul[@class='nav navbar-nav navbar-right navbar-nav-fancy']//li[@class='dropdown dropdown-avatar']"));
	}
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.id("username"));
		return isElementPresent(By.id("username"));
	}
}