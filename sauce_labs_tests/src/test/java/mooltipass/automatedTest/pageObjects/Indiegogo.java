package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Indiegogo extends AbstractPage{
	
	public Indiegogo (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//span[contains(text(),'Log In')]")
	private WebElement loginBtn;
	@FindBy(id = "account_email")
	private WebElement email;

	@FindBy(id = "account_password")
	private WebElement password;
	
	@FindBy(xpath = "//input[@value='Log In']")
	private WebElement submitLogin;
	
	
	@FindBy(xpath = "//a[contains(text(),'Log Out')]")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//a[@class='siteHeader-link siteHeader-link--name ng-scope']")
	private WebElement dashBoard;
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void submit(){

		waitUntilAppears(submitLogin);
		submitLogin.click();
	}
	public void goTodDashboard()
	{
		dashBoard.click();	
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath( "//a[@gogo-test='logged_in_name']"));
		return isElementPresent(By.xpath( "//a[@gogo-test='logged_in_name']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("account_email"));
	}

}
